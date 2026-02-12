#include "OARandomWalkSAT.h"
#include <random>
#include <limits>
#include "BitVectorNGenerator.h"

RandomWalkSAT::RandomWalkSAT(SATFormula formula, int maxTries, int maxFlips, double p) : formula(formula), maxTries(maxTries), maxFlips(maxFlips), p(p) {}

std::optional<BitVector> RandomWalkSAT::solve(const std::optional<BitVector>& initial) {
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<double> real_dist(0.0, 1.0);
    std::vector<bool> which(formula.getNumberOfClauses());

    for (int i = 0 ; i < maxTries ; i++) {
        BitVector assignment = BitVector(rng, formula.getNumberOfVariables());
        
        int best;

        for (int j = 0 ; j < maxFlips ; j++) {
            best = formula.whichSatisfied(assignment, which);
            if (best == formula.getNumberOfClauses()) return std::optional<BitVector>(assignment);

            if (real_dist(rng) < p) {   // Flip random in unsatisfied clause
                std::uniform_int_distribution<int> unsatisfied_dist(0, formula.getNumberOfClauses() - best - 1);
                int index = unsatisfied_dist(rng);  // Pick random unsatisfied clause
                int count = -1;
                for (std::size_t k = 0 ; k < which.size() ; k++) {
                    if (!which[k]) count++;
                    if (count == index) {   // Found the random unsatisfied clause
                        std::uniform_int_distribution<int> variable_dist(0, formula.getClause(k).getSize() - 1);
                        int literalToFlip = abs(formula.getClause(k).getLiteral(variable_dist(rng))) - 1;    // Pick random literal in chosen unsatisfied clause
                        MutableBitVector newAssignment = assignment.copy();
                        newAssignment.set(literalToFlip, !assignment.get(literalToFlip));
                        assignment = newAssignment;
                        break;
                    }
                }
            } else {    // Pick best neighbour
                best = -1;
                std::vector<size_t> bestIndices;
    
                std::vector<MutableBitVector> neighbourhood = BitVectorNGenerator(assignment).createNeighbourhood();
                for (std::size_t k = 0 ; k < neighbourhood.size() ; k++) {
                    int fitness = formula.nSatisfied(neighbourhood[k]);
                    if (fitness > best) {
                        best = fitness;
                        bestIndices.clear();
                        bestIndices.push_back(k);
                    } else if (fitness == best) {
                        bestIndices.push_back(k);
                    }
                }
    
                std::uniform_int_distribution<int> indices_dist(0, bestIndices.size() - 1);
                assignment = neighbourhood[indices_dist(rng)];
            }
        }
    }

    return std::optional<BitVector>();
}
