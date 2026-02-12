#pragma once
#include "IOptAlgorithm.h"
#include "SATFormula.h"

class GSAT : public IOptAlgorithm {
    private:
        SATFormula formula;
        int maxTries;
        int maxFlips;

    public:
        GSAT(SATFormula formula, int maxTries, int maxFlips);

        std::optional<BitVector> solve(const std::optional<BitVector>& initial);
};