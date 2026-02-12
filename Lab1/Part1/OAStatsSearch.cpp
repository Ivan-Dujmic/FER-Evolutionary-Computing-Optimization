#include "OAStatsSearch.h"
#include <random>
#include "BitVectorNGenerator.h"
#include <queue>
#include <algorithm>

StatsSearch::StatsSearch(SATFormula formula, int maxIterations, int numberOfBest, double percentageConstantUp, double percentageConstantDown, int percentageUnitAmount) :
formula(formula), maxIterations(maxIterations), numberOfBest(numberOfBest), percentageConstantUp(percentageConstantUp), percentageConstantDown(percentageConstantDown), percentageUnitAmount(percentageUnitAmount) {}

std::optional<BitVector> StatsSearch::solve(const std::optional<BitVector>& initial) {
    std::mt19937 rng(std::random_device{}());
    BitVector assignment = (initial.has_value()) ? initial.value() : BitVector(rng, formula.getNumberOfVariables());

    std::vector<double> post(formula.getNumberOfClauses()); // Keep statistics
    std::vector<bool> which(formula.getNumberOfClauses());  // Fill with bools correct and incorrect clauses

    // Comparator for min heap that keeps doubles of fitness and index of that neighbour
    auto cmp = [](const std::pair<double, std::size_t>& a, const std::pair<double, std::size_t>& b) {
        return a.first > b.first;
    };

    for (int i = 0 ; i < maxIterations ; i++) {
        if (formula.whichSatisfied(assignment, which) == formula.getNumberOfClauses()) return std::optional<BitVector>(assignment); // Found solution
        for (std::size_t i = 0 ; i < which.size() ; i++) {
            if (which[i]) post[i] += ((1 - post[i]) * percentageConstantUp);
            else post[i] -= (post[i] * percentageConstantDown);
        }
    
        std::priority_queue<std::pair<double, std::size_t>, std::vector<std::pair<double, std::size_t>>, decltype(cmp)> fitnesses(cmp); // Min heap
    
        std::vector<MutableBitVector> neighbourhood = BitVectorNGenerator(assignment).createNeighbourhood();
        for (size_t i = 0 ; i < neighbourhood.size() ; i++) {
            double fitness = formula.whichSatisfied(neighbourhood[i], which);
            for (size_t j = 0 ; j < which.size() ; j++) {
                if (which[j]) fitness += (percentageUnitAmount * (1 - post[j]));
                else fitness -= (percentageUnitAmount * (1 - post[j]));
            }
            
            fitnesses.emplace(fitness, i);
            if (fitnesses.size() > numberOfBest) fitnesses.pop();
        }
    
        // Pick random out of numberOfBest best neighbours
        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> dist(0, numberOfBest - 1);
        std::vector<size_t> bestIndices;
        bestIndices.reserve(numberOfBest);
        for (size_t i = 0 ; i < fitnesses.size() ; i++) {
            bestIndices.push_back(fitnesses.top().second);
            fitnesses.pop();
        }
        assignment = neighbourhood[bestIndices[dist(rng)]];
    }

    return std::optional<BitVector>();
}