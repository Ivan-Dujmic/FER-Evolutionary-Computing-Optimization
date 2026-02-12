#pragma once
#include "IOptAlgorithm.h"
#include "SATFormula.h"

class GreedyHillClimb : public IOptAlgorithm {
    private:
        SATFormula formula;
        int maxIterations;

    public:
        GreedyHillClimb(SATFormula formula, int maxIterations);

        std::optional<BitVector> solve(const std::optional<BitVector>& initial);
};