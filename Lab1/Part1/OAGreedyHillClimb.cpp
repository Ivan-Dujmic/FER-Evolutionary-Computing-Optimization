#include "OAGreedyHillClimb.h"
#include "MutableBitVector.h"
#include <random>
#include "BitVectorNGenerator.h"

GreedyHillClimb::GreedyHillClimb(SATFormula formula, int maxIterations) : formula(formula), maxIterations(maxIterations) {}

std::optional<BitVector> GreedyHillClimb::solve(const std::optional<BitVector>& initial) {
    std::mt19937 rng(std::random_device{}());
    BitVector assignment = (initial.has_value()) ? initial.value() : BitVector(rng, formula.getNumberOfVariables());

    int best = formula.nSatisfied(assignment);

    for (int i = 0 ; i < maxIterations ; i++) {
        std::vector<BitVector> candidates;

        for (BitVector neighbour : BitVectorNGenerator(assignment)) {
            int fitness = formula.nSatisfied(neighbour);
            if (fitness == formula.getNumberOfClauses()) return std::optional<BitVector>(neighbour);   // Found solution
            if (fitness >= best) {
                best = fitness;
                candidates.push_back(neighbour);
            }
        }

        if (candidates.empty()) return std::optional<BitVector>();  // Local optimum
        else {
            // Random better or equal neighbour is the new assignment
            std::uniform_int_distribution<int> dist(0, candidates.size() - 1);
            assignment = candidates[dist(rng)];
        }
    }

    return std::optional<BitVector>();  // No more iterations (possibly got stuck in a loop)
}