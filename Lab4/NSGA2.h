#pragma once
#include "MOOPProblem.h"
#include <random>
#include <limits>
#include <iostream>
#include <string>

struct Solution {
    std::vector<double> v;
    std::vector<double> fitness;
    std::size_t rank = 0;
    std::vector<Solution*> dominatingList;
    double crowdingDistance = 0;

    Solution() = default;
    explicit Solution(std::size_t vectorSize, std::size_t objectiveSize) : v(vectorSize), fitness(objectiveSize) {}

    friend std::ostream& operator<<(std::ostream& os, const Solution& u) {
        os << "Solution(fitness=[";
        for (std::size_t i = 0 ; i < u.fitness.size() ; i++) {
            os << u.fitness[i];
            if (i + 1 < u.fitness.size()) os << ", ";
        }
        os << "] v=[";
        for (std::size_t i = 0; i < u.v.size(); ++i) {
            os << u.v[i];
            if (i + 1 < u.v.size()) os << ", ";
        }
        os << "])";
        return os;
    }
};

class NSGA2 {
    private:
        std::size_t populationSize;
        double mutationChance;
        double mutationStrength;
        std::size_t iterations;

        MOOPProblem* problem;

        std::vector<Solution> population;
        std::vector<std::vector<Solution*>> fronts;
        bool requiresInit;

        std::mt19937 rng;
        std::vector<std::uniform_real_distribution<double>> boundaryDists;
        std::uniform_int_distribution<std::size_t> populationDist;
        std::uniform_real_distribution<double> chance;

        void nonDominatedSort();

        void calculateCrowdingDistances();

        void reducePopulation();

        Solution& select();

        Solution cross(const Solution& p1, const Solution& p2);

        void mutate(Solution& s);

        void expandPopulation();

    public:
        NSGA2(
            std::size_t populationSize,
            double mutationChance,
            double mutationStrength,
            std::size_t iterations,
            MOOPProblem* problem
        );

        void setProblem(MOOPProblem* problem);

        void initializePopulation();

        void train();

        void writeResults(std::string fileName);
};