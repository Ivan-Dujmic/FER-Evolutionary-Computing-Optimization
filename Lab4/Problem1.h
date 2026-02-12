#pragma once
#include "MOOPProblem.h"

class Problem1 : public MOOPProblem {
    public:
        Problem1(const std::vector<std::pair<double, double>>& boundaries);

        void evaluateSolution(const std::vector<double>& solution, std::vector<double>& fitness) const;
};