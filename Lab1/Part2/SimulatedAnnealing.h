#pragma once
#include <vector>
#include "ISystem.h"
#include <random>

class SimulatedAnnealing {
    private:
        bool maximize;  // true = maximize ; false = minimize
        int numOfVariables;
        ISystem* system;
        int maxIterations;
        int equilibrium;
        double initialTemp;
        double alpha;
        double neighbourMaxChange;
        std::mt19937 rng;

    public:
        SimulatedAnnealing(bool maximize, int numOfVariables, ISystem* system, int maxIterations, int equilibrium, double initialTemp, double alpha, double neighbourMaxChange);
        
        std::vector<double> getNeighbour(const std::vector<double>& base);
        std::vector<double> run();
};