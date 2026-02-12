#pragma once
#include "IOptAlgorithm.h"
#include "SATFormula.h"

class RandomWalkSAT : public IOptAlgorithm {
    private:
        SATFormula formula;
        int maxTries;
        int maxFlips;
        double p;   // Probability of random flip

    public:
        RandomWalkSAT(SATFormula formula, int maxTries, int maxFlips, double p);

        std::optional<BitVector> solve(const std::optional<BitVector>& initial);
};