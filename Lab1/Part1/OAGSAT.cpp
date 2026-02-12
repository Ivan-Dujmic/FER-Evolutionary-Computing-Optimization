#include "OAGSAT.h"
#include <random>
#include <limits>
#include "BitVectorNGenerator.h"

GSAT::GSAT(SATFormula formula, int maxTries, int maxFlips) : formula(formula), maxTries(maxTries), maxFlips(maxFlips) {}

std::optional<BitVector> GSAT::solve(const std::optional<BitVector>& initial) {
    std::mt19937 rng(std::random_device{}());
    
    for (int i = 0 ; i < maxTries ; i++) {
        BitVector assignment = BitVector(rng, formula.getNumberOfVariables());
        if (formula.isSatisfied(assignment)) return std::optional<BitVector>(assignment);
        
        for (int j = 0 ; j < maxFlips ; j++) {
            int best = -1;
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

            std::uniform_int_distribution<int> dist(0, bestIndices.size() - 1);
            assignment = neighbourhood[bestIndices[dist(rng)]];
            if (best == formula.getNumberOfClauses()) return std::optional<BitVector>(assignment);
        }
    }

    return std::optional<BitVector>();
}
