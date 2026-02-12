#include "OAPerturbatingILS.h"
#include "MutableBitVector.h"
#include <random>
#include "BitVectorNGenerator.h"

PerturbatingILS::PerturbatingILS(SATFormula formula, int maxIterations, double toChange) : formula(formula), maxIterations(maxIterations), toChange(formula.getNumberOfVariables() * toChange) {}

std::optional<BitVector> PerturbatingILS::solve(const std::optional<BitVector>& initial) {
    std::mt19937 rng(std::random_device{}());
    BitVector assignment = (initial.has_value()) ? initial.value() : BitVector(rng, formula.getNumberOfVariables());
    std::vector<int> shuffleDeck(formula.getNumberOfVariables());
    std::iota(shuffleDeck.begin(), shuffleDeck.end(), 0);

    int best = formula.nSatisfied(assignment);
    if (best == formula.getNumberOfClauses()) return std::optional<BitVector>(assignment);  // Found solution

    for (int i = 0 ; i < maxIterations ; i++) {
        std::vector<BitVector> candidates;

        for (BitVector neighbour : BitVectorNGenerator(assignment)) {
            int fitness = formula.nSatisfied(neighbour);
            if (fitness == formula.getNumberOfClauses()) return std::optional<BitVector>(neighbour);   // Found solution
            if (fitness > best) {
                best = fitness;
                candidates.push_back(neighbour);
            }
        }

        if (candidates.empty()) {   // Local optimum
            std::shuffle(shuffleDeck.begin(), shuffleDeck.end(), rng);
            MutableBitVector newAssignment = assignment.copy();
            for (int j = 0 ; j < toChange ; j++) {
                newAssignment.set(shuffleDeck[j], !newAssignment.get(shuffleDeck[j]));
            }
            assignment = newAssignment;
            best = formula.nSatisfied(assignment);
            if (best == formula.getNumberOfClauses()) return std::optional<BitVector>(assignment);  // Found solution
        } else {
            // Random better or equal neighbour is the new assignment
            std::uniform_int_distribution<int> dist(0, candidates.size() - 1);
            assignment = candidates[dist(rng)];
        }
    }

    return std::optional<BitVector>();
}