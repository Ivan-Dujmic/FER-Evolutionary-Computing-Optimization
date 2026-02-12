#include "SimulatedAnnealing.h"
#include <numbers>
#include <cmath>
#include <iostream>

SimulatedAnnealing::SimulatedAnnealing(bool maximize, int numOfVariables, ISystem* system, int maxIterations, int equilibrium, double initialTemp, double alpha, double neighbourMaxChange)
: maximize(maximize), numOfVariables(numOfVariables), system(system), maxIterations(maxIterations), equilibrium(equilibrium), initialTemp(initialTemp), alpha(alpha), neighbourMaxChange(neighbourMaxChange), rng(std::random_device{}()) {}

std::vector<double> SimulatedAnnealing::getNeighbour(const std::vector<double>& base) {
    static std::uniform_real_distribution dist(-neighbourMaxChange, neighbourMaxChange);

    std::vector<double> neighbour = base;
    for (double& param : neighbour) {
        param += dist(rng);
    }    

    return neighbour;
}

std::vector<double> SimulatedAnnealing::run() {
    static std::uniform_real_distribution dist(0.0, 1.0);
    static std::uniform_real_distribution init_dist(-10.0, 10.0);

    std::vector<double> solution(numOfVariables);
    for (double& var : solution) {
        var += init_dist(rng);
    }
    std::vector<double> bestSolution = solution;
    double best = system->getOptimizationParameter(bestSolution);
    double t = initialTemp;

    for (int i = 0 ; i < maxIterations ; i++) {
        for (int j = 0 ; j < equilibrium ; j++) {
            std::vector<double> neighbour = getNeighbour(solution);
            double neighbourFitness = system->getOptimizationParameter(neighbour);
            double change = neighbourFitness - system->getOptimizationParameter(solution);
            if (maximize) {
                if (change > 0) {
                    solution = neighbour;
                    if (neighbourFitness < best) {
                        bestSolution = solution;
                        best = neighbourFitness;
                    }
                } else solution = (dist(rng) < std::pow((std::numbers::e), -change / t)) ? neighbour : solution;
            } else {
                if (change < 0) {
                    solution = neighbour;
                    if (neighbourFitness < best) {
                        bestSolution = solution;
                        best = neighbourFitness;
                    }
                } else solution = (dist(rng) < std::pow((std::numbers::e), -change / t)) ? neighbour : solution;
            }
        }
        t *= alpha;

        std::cout << "Best error so far: " << best << '\n';
    }

    bestSolution.push_back(best);
    return bestSolution;
}
