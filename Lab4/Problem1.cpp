#include "Problem1.h"

Problem1::Problem1(const std::vector<std::pair<double, double>>& boundaries) : MOOPProblem(4, std::move(boundaries)) {}

void Problem1::evaluateSolution(const std::vector<double>& solution, std::vector<double>& fitness) const {
    for (std::size_t i = 0 ; i <= 4 ; i++) {
        fitness[i] = solution[i] * solution[i];
    }
}