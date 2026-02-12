#pragma once
#include "MOOPProblem.h"

class Problem2 : public MOOPProblem {
    public:
        Problem2(const std::vector<std::pair<double, double>>& boundaries);

        void evaluateSolution(const std::vector<double>& solution, std::vector<double>& fitness) const;
};