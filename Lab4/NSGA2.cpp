#include "NSGA2.h"
#include <iostream>
#include <algorithm>
#include <fstream>

NSGA2::NSGA2(
    std::size_t populationSize,
    double mutationChance,
    double mutationStrength,
    std::size_t iterations,
    MOOPProblem* problem
) :
populationSize(populationSize),
mutationChance(mutationChance),
mutationStrength(mutationStrength),
iterations(iterations),
rng(std::random_device{}()),
populationDist(0, populationSize - 1),
chance(0, 1) {
    this->setProblem(problem);
}

void NSGA2::nonDominatedSort() {
    this->fronts.clear();
    for (Solution& s : this->population) {
        s.rank = 0;
    }

    for (std::size_t i = 0 ; i < this->population.size() ; i++) {
        Solution& a = this->population[i];
        a.dominatingList.clear();
        for (std::size_t j = 0 ; j < this->population.size() ; j++) {
            if (i == j) continue;
            Solution& b = this->population[j];
            // Check if i-th dominates j-th
            bool worse = false;
            bool better = false; // Marks if we've found at least one that's strictly better (not just equal)
            for (std::size_t k = 0 ; k < this->problem->getNumberOfObjectives() ; k++) {
                double aFit = a.fitness[k];
                double bFit = b.fitness[k];
                if (aFit > bFit) {
                    worse = true;
                    break;
                } else if (aFit < bFit) {
                    better = true;
                }
            }
            if (!worse && better) {
                a.dominatingList.push_back(&b);
                b.rank++;
            }
        }
    }

    std::vector<Solution*> populationPointers;
    populationPointers.reserve(this->population.size());
    for (Solution& s : this->population) {
        populationPointers.push_back(&s);
    }

    // We can stop forming fronts once we've got enough to fill our population
    std::size_t added = 0;
    for (std::size_t i = 0 ; added < this->populationSize ; i++) {
        this->fronts.emplace_back();
        
        auto it = std::partition(
            populationPointers.begin(),
            populationPointers.end(),
            [](Solution* s){ return s->rank != 0; }
        );
        this->fronts[i].insert(
            this->fronts[i].end(),
            it,
            populationPointers.end()
        );
        populationPointers.erase(it, populationPointers.end());

        for (Solution* s : this->fronts[i]) {
            s->crowdingDistance = 0;
            s->rank = i;
            for (Solution* d : s->dominatingList) {
                d->rank--;
            }
        }

        added += this->fronts[i].size();
    }
}

void NSGA2::calculateCrowdingDistances() {
    for (auto& front : this->fronts) {
        for (std::size_t i = 0 ; i < this->problem->getNumberOfObjectives() ; i++) {
            std::sort(
                front.begin(),
                front.end(),
                [i](Solution* a, Solution* b){ return a->fitness[i] < b->fitness[i]; }
            );

            front[0]->crowdingDistance = std::numeric_limits<double>::infinity();
            front[front.size() - 1]->crowdingDistance = std::numeric_limits<double>::infinity();
            double maxDiff = front[front.size() - 1]->fitness[i] - front[0]->fitness[i];
        
            if (maxDiff == 0) {
                for (std::size_t j = 1 ; j < front.size() - 1 ; j++) {
                    front[j]->crowdingDistance = std::numeric_limits<double>::infinity();
                }
            }

            for (std::size_t j = 1 ; j < front.size() - 1 ; j++) {
                front[j]->crowdingDistance += (front[j+1]->fitness[i] - front[j-1]->fitness[i]) / maxDiff;
            }
        }
    }
}

void NSGA2::reducePopulation() {
    std::vector<Solution> newPopulation;
    newPopulation.reserve(this->populationSize);

    for (auto& front : this->fronts) {
        std::size_t missing = this->populationSize - newPopulation.size();
        if (front.size() <= missing) {
            for (Solution* s : front) {
                newPopulation.push_back(*s);
            }
        } else {
            sort(
                front.begin(),
                front.end(),
                [](Solution* a, Solution* b){ return a->crowdingDistance > b->crowdingDistance; }
            );
            for (std::size_t i = 0 ; i < missing ; i++) {
                newPopulation.push_back(*front[i]);
            }
            break;
        }
    }

    this->population = std::move(newPopulation);
}

Solution& NSGA2::select() {
    Solution* winner = &(this->population[this->populationDist(this->rng)]);
    for (std::size_t i = 0 ; i < 2 ; i++) {
        Solution* chosen = &(this->population[this->populationDist(this->rng)]);
        if (chosen->rank < winner->rank || (chosen->rank == winner->rank && chosen->crowdingDistance > winner->crowdingDistance)) {
            winner = chosen;
        }
    }

    return *winner;
}

Solution NSGA2::cross(const Solution& p1, const Solution& p2) {
    Solution result(this->problem->getDimensions(), this->problem->getNumberOfObjectives());
    for (std::size_t i = 0 ; i < this->problem->getDimensions() ; i++) {
        double min = std::min(p1.v[i], p2.v[i]);
        double max = std::max(p1.v[i], p2.v[i]);
        std::uniform_real_distribution<double> crossDist(min, max);
        result.v[i] = crossDist(this->rng);
    }
    return result;
}

// Without clamp
// void NSGA2::mutate(Solution& s) {
//     for (std::size_t i = 0 ; i < this->problem->getDimensions() ; i++) {
//         if (this->chance(this->rng) > this->mutationChance) {
//             continue;
//         }
        
//         double min = s.v[i] - (s.v[i] - this->problem->getBoundary(i).first) * this->mutationStrength;
//         double max = s.v[i] + (this->problem->getBoundary(i).second - s.v[i]) * this->mutationStrength; 
//         std::uniform_real_distribution<double> mutationDist(min, max);
//         s.v[i] = mutationDist(this->rng);
//     }
// }

// With clamp
void NSGA2::mutate(Solution& s) {
    for (std::size_t i = 0 ; i < this->problem->getDimensions() ; i++) {
        if (this->chance(this->rng) > this->mutationChance) {
            continue;
        }

        double lowerBound = this->problem->getBoundary(i).first;
        double upperBound = this->problem->getBoundary(i).second;

        double diff = upperBound - lowerBound;
        
        double min = s.v[i] - diff;
        double max = s.v[i] + diff;
        std::uniform_real_distribution<double> mutationDist(min, max);
        s.v[i] = mutationDist(this->rng);
        if (s.v[i] < lowerBound) s.v[i] = lowerBound;
        else if (s.v[i] > upperBound) s.v[i] = upperBound;
    }
}

void NSGA2::expandPopulation() {
    for (std::size_t i = 0 ; i < this->populationSize ; i++) {
        Solution child = this->cross(this->select(), this->select());
        this->mutate(child);
        this->problem->evaluateSolution(child.v, child.fitness);
        this->population.push_back(child);
    }
}

void NSGA2::setProblem(MOOPProblem* problem) {
    this->problem = problem;
    this->boundaryDists.clear();
    for (std::size_t i = 0 ; i < this->problem->getNumberOfObjectives() ; i++) {
        std::pair<double, double> boundaryPair = this->problem->getBoundary(i);
        this->boundaryDists.push_back(std::uniform_real_distribution<double>(boundaryPair.first, boundaryPair.second));
    }
    this->requiresInit = true;
}

void NSGA2::initializePopulation() {
    this->population = std::vector<Solution>(this->populationSize, Solution(this->problem->getDimensions(), this->problem->getNumberOfObjectives()));

    for (Solution& solution : population) {
        for (std::size_t i = 0 ; i < this->problem->getDimensions() ; i++) {
            solution.v[i] = this->boundaryDists[i](this->rng);
        }
        this->problem->evaluateSolution(solution.v, solution.fitness);
    }

    this->requiresInit = false;
}

void NSGA2::train() {
    if (this->requiresInit) {
        std::cerr << "Population initialization required!\n";
        return;
    }

    this->nonDominatedSort();
    this->calculateCrowdingDistances();
    for (std::size_t i = 0 ; i < this->iterations ; i++) {
        this->expandPopulation();
        this->nonDominatedSort();
        this->calculateCrowdingDistances();
        this->reducePopulation();
    }

    this->nonDominatedSort();
}

void NSGA2::writeResults(std::string fileName) {
    std::ofstream file(fileName);

    file << "Front sizes:\n";
    for (std::size_t i = 0 ; i < this->fronts.size() ; i++) {
        file << i << " - " << this->fronts[i].size() << '\n';
    }
    file << "\nF0 solutions:\n";
    for (Solution* s : this->fronts[0]) {
        file << *s << '\n';
    }

    file.close();
}