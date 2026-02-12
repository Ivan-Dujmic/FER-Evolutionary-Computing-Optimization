#pragma once
#include "IOptAlgorithm.h"
#include "SATFormula.h"

class StatsSearch : public IOptAlgorithm {
    private:
        SATFormula formula;
        int maxIterations;
        int numberOfBest;
        double percentageConstantUp;
        double percentageConstantDown;
        int percentageUnitAmount;

    public:
        StatsSearch(SATFormula formula, int maxIterations, int numberOfBest, double percentageConstantUp, double percentageConstantDown, int percentageUnitAmount);

        std::optional<BitVector> solve(const std::optional<BitVector>& initial);
};