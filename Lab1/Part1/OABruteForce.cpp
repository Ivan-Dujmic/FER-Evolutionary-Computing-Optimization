#include "OABruteForce.h"
#include "MutableBitVector.h"
#include <iostream>

BruteForce::BruteForce(SATFormula formula, bool printAll) : formula(formula), printAll(printAll) {}

std::optional<BitVector> BruteForce::solve(const std::optional<BitVector>& initial) {
    MutableBitVector assignment = MutableBitVector(formula.getNumberOfVariables()); // Start at zero vector
    std::optional<BitVector> solution;
    
    do {
        if (formula.isSatisfied(assignment)) {
            solution = assignment;
            if (printAll) std::cout << assignment.toString() << '\n';
        }
    } while (!assignment.increment());    // Until overflow

    return solution;
}