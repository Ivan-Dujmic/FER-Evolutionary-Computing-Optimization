#pragma once
#include "IOptAlgorithm.h"
#include "SATFormula.h"

class PerturbatingILS : public IOptAlgorithm {
    private:
        SATFormula formula;
        int maxIterations;
        int toChange;

    public:
        PerturbatingILS(SATFormula formula, int maxIterations, double toChange);    //  toChange - Percent [0, 1] of random variables to flip when stuck

        std::optional<BitVector> solve(const std::optional<BitVector>& initial);
};