#include <vector>
#include <memory>
#include <string>
#include <functional>
#include <random>
#include <iostream>

template <typename T, typename Context>
class GPNode;

template <typename T, typename Context>
class GP;

template <typename T, typename Context>
using GPFunc = std::function<T(std::string* symbol, const std::vector<std::unique_ptr<GPNode<T, Context>>>&, const Context&)>;

template <typename T, typename Context>
struct GPOperator {
    std::string symbol;
    GPFunc<T, Context> func;
    std::size_t arity;
};

template <typename T, typename Context>
class GPNode {
    friend class GP<T, Context>;

    private:
        GPOperator<T, Context> oper;
        GPNode<T, Context>* parent;
        std::vector<std::unique_ptr<GPNode<T, Context>>> children;
        std::size_t subtreeSize;
        std::size_t subtreeDepth;
        double penalty = -1;    // Only necessary for root nodes

        std::mt19937 rng;

        std::size_t recalculateSubtreeSizesRecursive() {
            std::size_t sum = 1;
            std::size_t maxDepth = 0;
            for (auto& child : this->children) {
                sum += child->recalculateSubtreeSizesRecursive();
                if (child->getSubtreeDepth() > maxDepth) maxDepth = child->getSubtreeDepth();
            }
            this->subtreeSize = sum;
            this->subtreeDepth = maxDepth + 1;
            return sum;
        }
        
    public:
        GPNode(GPOperator<T, Context> oper, GPNode<T, Context>* parent = nullptr)
        : oper(oper), parent(parent), rng(std::random_device{}()) {children.resize(oper.arity);}

        // Constructor for cloning
        GPNode(const GPNode<T, Context>* node, GPNode<T, Context>* parent = nullptr) : 
            oper(node->oper),
            parent(parent),
            subtreeSize(node->subtreeSize),
            subtreeDepth(node->subtreeDepth), 
            penalty(node->penalty)
            {children.resize(node->oper.arity);} 

        std::unique_ptr<GPNode<T, Context>> clone(GPNode<T, Context>* parent = nullptr) {
            auto newNode = std::make_unique<GPNode<T, Context>>(this, parent);

            std::size_t index = 0;
            for (const auto& child : this->children) {
                newNode->setChild(index, child->clone(newNode.get()));
                index++;
            }

            return newNode;
        }

        std::string& getSymbol() {
            return oper.symbol;
        }

        std::size_t getArity() {
            return oper.arity;
        }

        GPNode<T, Context>* getParent() {
            return parent;
        }

        void setParent(GPNode<T, Context>* parent) {
            this->parent = parent;
        } 

        GPNode<T, Context>* getChild(std::size_t index) {
            return children[index].get();
        }

        void setChild(std::size_t index, std::unique_ptr<GPNode<T, Context>> child) {
            children[index] = std::move(child);
        }

        const std::vector<std::unique_ptr<GPNode<T, Context>>>& getChildren() {
            return children;
        }

        std::size_t getSubtreeSize() {
            return subtreeSize;
        }

        std::size_t getSubtreeDepth() {
            return subtreeDepth;
        }

        double getPenalty() {
            return penalty;
        }

        void setPenalty(double penalty) {
            this->penalty = penalty;
        }

        // Recalculates subtree sizes for this node, all it's children and the parent path until root
        void recalculateSubtreeSizes() {
            // Down
            recalculateSubtreeSizesRecursive();

            // Up
            GPNode<T, Context>* node = parent;
            while (node != nullptr) {
                std::size_t sum = 1;
                std::size_t maxDepth = 0;
                for (const auto& child : node->children) {
                    sum += child->getSubtreeSize();
                    if (child->getSubtreeDepth() > maxDepth) maxDepth = child->getSubtreeDepth();
                }
                node->subtreeSize = sum;
                node->subtreeDepth = maxDepth + 1;

                node = node->getParent();
            }
        }

        // Returns the randomly chosen node and it's index in it's parent's children list
        std::pair<GPNode<T, Context>*, std::size_t> getRandomNode() {
            std::uniform_int_distribution<std::size_t> dist(1, this->subtreeSize - 1); 
            std::size_t budget = dist(rng);

            GPNode<T, Context>* node = this;
            std::size_t childIndex;

            while (budget != 0) {
                childIndex = 0;
                for (const auto& child : node->getChildren()) {
                    if (budget > child->getSubtreeSize()) budget -= child->getSubtreeSize();
                    else {
                        budget--;
                        node = child.get();
                        break;
                    }
                    childIndex++;
                }
            }

            return std::make_pair(node, childIndex);
        }
        
        T evaluate(const Context& context) {
            return oper.func(&oper.symbol, children, context);
        }

        std::string toString() {
            switch (this->oper.arity) {
                case 0:
                    if (this->oper.symbol == "") return "C";
                    else return this->oper.symbol;
                case 1:
                    return this->oper.symbol + "(" + this->children[0]->toString() + ")";
                case 2:
                    return "(" + this->children[0]->toString() + this->oper.symbol + this->children[1]->toString() + ")"; 
            }

            return "";
        }
};

template <typename T, typename Context>
class IPenalty {
    public:
        virtual double calculate(std::vector<Context>& contexts, GPNode<T, Context>& tree) = 0; 
};

template <typename T, typename Context>
class GP {
    private:
        std::vector<std::unique_ptr<GPNode<T, Context>>> population;
        std::size_t populationSize;

        std::vector<GPOperator<T, Context>> operators;  // Sorted by arity; Terminals are also operators, but with arity 0
        std::vector<std::size_t> operCountUpToArity;    // Number of operators of arity that's less or equal than index...
        std::size_t minNonZeroArity = -1;
        std::size_t maxArity;
        /*
        For FULL init method:
        (budget-1)/GeometricSeries(minArity, depth-2) >= arity_of_current_node
        */
        std::vector<std::size_t> minArityGeometricSeries;
        std::size_t maxDepthFull;

        std::size_t costEvaluations;
        std::size_t maxTreeDepth;
        std::size_t maxTreeNodes;
        std::size_t tournamentSize;
        std::size_t elitism;
        double pClone;
        double pMutate;
        double pCross;

        IPenalty<T, Context>* penalty;

        std::vector<Context> contexts;

        std::mt19937 rng;
        std::uniform_real_distribution<double> probDist;

        std::unique_ptr<GPNode<T, Context>> full(std::size_t maxDepth, std::size_t maxNodes, GPNode<T, Context>* parent = nullptr) {
            // Find the highest arity we can use for this node and still be able to generate a full tree if using the lowest non-zero arity
            std::size_t arity = -1;
            if (maxDepth == 1) arity = 0;
            else {
                for (std::size_t i = this->maxArity ; i >= this->minNonZeroArity ; i--) {
                    // If there exists at least one operator of this arity
                    if (this->operCountUpToArity[i] - this->operCountUpToArity[i-1] > 0) {
                        if (maxNodes - 1 >= i * this->minArityGeometricSeries[maxDepth - 2]) {
                            arity = i;
                            break;
                        }
                    }
                }
            }
            std::uniform_int_distribution<std::size_t> distArities;
            if (arity == 0) {
                distArities = std::uniform_int_distribution<std::size_t>(0, this->operCountUpToArity[0] - 1);                
            } else {
                distArities = std::uniform_int_distribution<std::size_t>(this->operCountUpToArity[0], this->operCountUpToArity[arity] - 1);
            }
            GPOperator<T, Context>& oper = this->operators[distArities(rng)];
            std::unique_ptr<GPNode<T, Context>> node = std::make_unique<GPNode<T, Context>>(oper, parent);
            if (maxDepth == 1) return node;
            /*  
                Split the budget (maxNodes) between children nodes
                Each child needs a budget of at least GeometricSeries[child's maxDepth - 1] so we reduce by that * oper.arity
                Add one because we are splitting the budget and the splitters can be placed anywhere between two coins or outside,
                but remove one cause we used it on this node,
                but remove another one because we start from zero
            */
           std::uniform_int_distribution<std::size_t> distCounts(0, maxNodes - oper.arity * this->minArityGeometricSeries[maxDepth - 2] - 1);
           std::vector<std::size_t> splitters = {0, maxNodes - oper.arity * this->minArityGeometricSeries[maxDepth - 2] - 1};
            for (std::size_t i = 0 ; i < oper.arity - 1 ; i++) {
                splitters.push_back(distCounts(rng));
            }
            std::sort(splitters.begin(), splitters.end());
            for (std::size_t i = 0 ; i < splitters.size() - 1 ; i++) {
                node->setChild(i, this->full(maxDepth - 1, this->minArityGeometricSeries[maxDepth - 2] + splitters[i+1] - splitters[i], node.get()));
            }
            return node;
        }
        
        std::unique_ptr<GPNode<T, Context>> grow(std::size_t maxDepth, std::size_t maxNodes, GPNode<T, Context>* parent = nullptr, bool banTerminals = false) {
            std::size_t arity = this->maxArity;
            // Max arity this node's operator can be based on the leftover budget (maxNodes)
            if (maxNodes < this->maxArity + 1) arity = maxNodes - 1;
            if (maxDepth == 1) arity = 0;
            std::uniform_int_distribution<std::size_t> distArities;
            if (banTerminals) {
                distArities = std::uniform_int_distribution<std::size_t>(this->operCountUpToArity[0], this->operCountUpToArity[arity] - 1);
            } else {
                distArities = std::uniform_int_distribution<std::size_t>(0, this->operCountUpToArity[arity] - 1);
            }
            GPOperator<T, Context> oper = this->operators[distArities(rng)];
            std::unique_ptr<GPNode<T, Context>> node = std::make_unique<GPNode<T, Context>>(oper, parent);
            if (maxDepth == 1 || oper.arity == 0) return node;     // Not necessary, but helps
            /*  
                Split the budget (maxNodes) between children nodes
                Each child needs a budget of at least 1 so we reduce by oper.arity
                Add one because we are splitting the budget and the splitters can be placed anywhere between two coins or outside,
                but remove one cause we used it on this node,
                but remove another one because we start from zero
            */
            std::uniform_int_distribution<std::size_t> distCounts(0, maxNodes - oper.arity - 1);
            std::vector<std::size_t> splitters = {0, maxNodes - oper.arity - 1};
            if (oper.arity != 0) {
                for (std::size_t i = 0 ; i < oper.arity - 1 ; i++) {
                    splitters.push_back(distCounts(rng));
                }
            }
            std::sort(splitters.begin(), splitters.end());
            for (std::size_t i = 0 ; i < splitters.size() - 1 ; i++) {
                node->setChild(i, this->grow(maxDepth - 1, 1 + splitters[i+1] - splitters[i], node.get()));
            }
            return node;
        }

        std::unique_ptr<GPNode<T, Context>> mutate(GPNode<T, Context>* tree) {
            std::unique_ptr<GPNode<T, Context>> mutatedTree = tree->clone();
            auto p = mutatedTree->getRandomNode();
            GPNode<T, Context>* subtreeToRemove = p.first;
            std::size_t removalIndex = p.second;

            std::unique_ptr<GPNode<T, Context>> newSubtree = this->grow(
                this->maxTreeDepth - mutatedTree->getSubtreeDepth() + subtreeToRemove->getSubtreeDepth(),
                this->maxTreeNodes - mutatedTree->getSubtreeSize() + subtreeToRemove->getSubtreeSize(),
                subtreeToRemove->getParent()
            );

            subtreeToRemove->getParent()->setChild(removalIndex, std::move(newSubtree));
            mutatedTree->recalculateSubtreeSizes();

            mutatedTree->setPenalty(this->penalty->calculate(this->contexts, *mutatedTree));
            this->costEvaluations++;

            return mutatedTree;
        }

        // If crossing fails for a child (exceeded max depth or node count) then return nullptr
        std::pair<std::unique_ptr<GPNode<T, Context>>, std::unique_ptr<GPNode<T, Context>>> cross(GPNode<T, Context>* parent1, GPNode<T, Context>* parent2) {
            std::unique_ptr<GPNode<T, Context>> child1 = parent1->clone();
            std::unique_ptr<GPNode<T, Context>> child2 = parent2->clone();

            auto p1 = child1->getRandomNode();
            auto p2 = child2->getRandomNode();

            // Switch nodes between trees
            std::swap(p1.first->getParent()->children[p1.second], p2.first->getParent()->children[p2.second]);

            // Fix parents
            GPNode<T, Context>* tmp = p1.first->getParent();
            p1.first->setParent(p2.first->getParent());
            p2.first->setParent(tmp);

            bool child1Invalid = child1->getSubtreeDepth() - p1.first->getSubtreeDepth() + p2.first->getSubtreeDepth() > this->maxTreeDepth
                            ||  child1->getSubtreeSize() - p1.first->getSubtreeSize() + p2.first->getSubtreeSize() > this->maxTreeNodes;

            bool child2Invalid = child2->getSubtreeDepth() - p2.first->getSubtreeDepth() + p1.first->getSubtreeDepth() > this->maxTreeDepth
                            ||  child2->getSubtreeSize() - p2.first->getSubtreeSize() + p1.first->getSubtreeSize() > this->maxTreeNodes;

            if (child1Invalid && child2Invalid) {
                return std::make_pair(nullptr, nullptr);
            }

            if (child1Invalid) {
                child2->recalculateSubtreeSizes();
                child2->setPenalty(this->penalty->calculate(this->contexts, *child2));
                this->costEvaluations++;
                return std::make_pair(std::move(child2), nullptr);
            }

            if (child2Invalid) {
                child1->recalculateSubtreeSizes();
                child1->setPenalty(this->penalty->calculate(this->contexts, *child1));
                this->costEvaluations++;
                return std::make_pair(std::move(child1), nullptr);
            }

            child1->recalculateSubtreeSizes();
            child2->recalculateSubtreeSizes();
            child1->setPenalty(this->penalty->calculate(this->contexts, *child1));
            child2->setPenalty(this->penalty->calculate(this->contexts, *child2));
            this->costEvaluations += 2;
            return std::make_pair(std::move(child1), std::move(child2));
        }

        std::vector<GPNode<T, Context>*> tournament(std::size_t winnersCount) {
            std::shuffle(population.begin(), population.end(), rng);
            std::vector<GPNode<T, Context>*> competitors;
            for (std::size_t i = 0 ; i < this->tournamentSize ; i++) {
                competitors.push_back(population[i].get());
            }

            std::sort(competitors.begin(), competitors.end(),
                [](const auto& a, const auto& b) {
                    return a->getPenalty() < b->getPenalty();
                }
            );

            return std::vector<GPNode<T, Context>*>(competitors.begin(), competitors.begin() + winnersCount);
        }

    public:
        GP(
            std::vector<GPOperator<T, Context>> operators,
            std::size_t populationSize,
            std::size_t maxTreeDepth,
            std::size_t maxTreeNodes,
            std::size_t tournamentSize,
            std::size_t elitism,
            double pClone,
            double pMutate,
            double pCross,
            IPenalty<T, Context>* penalty,
            std::vector<Context> contexts
        ) :
        populationSize(populationSize),
        costEvaluations(0),
        maxTreeDepth(maxTreeDepth),
        maxTreeNodes(maxTreeNodes),
        tournamentSize(tournamentSize),
        elitism(elitism),
        pClone(pClone),
        pMutate(pMutate),
        pCross(pCross),
        penalty(penalty),
        contexts(contexts),
        rng(std::random_device{}()),
        probDist(0.0, 1.0) {
            this->population.resize(populationSize);

            this->operators = operators;
            std::sort(this->operators.begin(), this->operators.end(),
                [](auto const& a, auto const& b) {
                    return a.arity < b.arity;
                }
            );
            
            this->maxArity = this->operators.back().arity;

            this->operCountUpToArity.resize(this->maxArity + 1);
            for (const auto& oper : this->operators) {
                this->operCountUpToArity[oper.arity]++;
                if (this->minNonZeroArity == -1 && oper.arity != 0) this->minNonZeroArity = oper.arity;
            }

            std::size_t count = 0;
            for (std::size_t i = 0 ; i <= this->maxArity ; i++) {
                this->operCountUpToArity[i] += count;
                count = this->operCountUpToArity[i];
            }

            std::size_t geoSum = 1;
            minArityGeometricSeries.push_back(geoSum);
            for (std::size_t N = 1; N < 20 ; N++) {
                geoSum *= this->minNonZeroArity;
                this->minArityGeometricSeries.push_back(this->minArityGeometricSeries[N-1] + geoSum);
            }

            // Find the highest depth for full init (using only minimal non-zero arity operators)
            for (std::size_t d = this->maxTreeDepth ; d > 1 ; d--) {
                if (this->maxTreeNodes >= this->minArityGeometricSeries[d-1]) {
                    this->maxDepthFull = d;
                    break;
                }
            }
        }

        std::size_t getCostEvaluations() {
            return this->costEvaluations;
        }

        GPNode<T, Context>& getBestSolution() {
            auto bestIt = std::min_element(
                this->population.begin(),
                this->population.end(),
                [](const std::unique_ptr<GPNode<T, Context>>& a,
                const std::unique_ptr<GPNode<T, Context>>& b) {
                    return a->getPenalty() < b->getPenalty();
                }
            );

            return *bestIt->get();
        }

        void initializePopulation() {   // Ramped half-and-half
            for (std::size_t i = this->populationSize / 2 ; i < this->populationSize ; i++) {
                population[i] = this->grow(2 + i % (this->maxTreeDepth - 1), this->maxTreeNodes, nullptr, true); // Depth is in [0, maxTreeDepth]
            }

            for (std::size_t i = 0; i < this->populationSize / 2 ; i++) {
                population[i] = this->full(2 + i % (this->maxDepthFull - 1), this->maxTreeNodes); // Depth is in [0, maxDepthFull]
            }

            for (const auto& node : population) {
                node->recalculateSubtreeSizes();
                node->setPenalty(this->penalty->calculate(this->contexts, *node));
                this->costEvaluations++;
            }
        }

        void newGeneration() {
            std::vector<std::unique_ptr<GPNode<T, Context>>> newPopulation;

            // Sort by fitness for elitism
            std::sort(this->population.begin(), this->population.end(),
                [](const auto& a, const auto& b) {
                    return a->getPenalty() < b->getPenalty();
                }
            );

            for (std::size_t i = 0 ; i < this->elitism ; i++) {
                newPopulation.push_back(this->population[i]->clone());
            }

            while (newPopulation.size() < this->populationSize) {
                double roll = probDist(rng);
                if (roll < this->pClone) {  // Clone
                    newPopulation.push_back(std::move(this->tournament(1)[0]->clone()));
                } else if (roll < this->pClone + this->pMutate) {   // Mutate
                    newPopulation.push_back(std::move(this->mutate(this->tournament(1)[0])));
                } else {    // Cross
                    auto parents = this->tournament(2);
                    auto p = this->cross(parents[0], parents[1]);
                    if (p.first != nullptr) {
                        newPopulation.push_back(std::move(p.first));
                    }
                    if (newPopulation.size() < this->populationSize && p.second != nullptr) {
                        newPopulation.push_back(std::move(p.second));
                    }
                }
            }

            this->population = std::move(newPopulation);
        }
};