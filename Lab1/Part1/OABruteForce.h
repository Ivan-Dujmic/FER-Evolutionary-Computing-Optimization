#pragma once
#include "IOptAlgorithm.h"
#include "SATFormula.h"

class BruteForce : public IOptAlgorithm {
    private:
        SATFormula formula;
        bool printAll;

    public:
        BruteForce(SATFormula formula, bool printAll);

        std::optional<BitVector> solve(const std::optional<BitVector>& initial);
};