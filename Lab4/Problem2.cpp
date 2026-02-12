#include "Problem2.h"

Problem2::Problem2(const std::vector<std::pair<double, double>>& boundaries) : MOOPProblem(2, std::move(boundaries)) {}

void Problem2::evaluateSolution(const std::vector<double>& solution, std::vector<double>& fitness) const {
    fitness[0] = solution[0];
    fitness[1] = (1 + solution[1]) / solution[0];
}