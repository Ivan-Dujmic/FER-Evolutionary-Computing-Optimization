#pragma once
#include "ISystem.h"
#include <random>
#include <limits>
#include <iostream>

struct Particle {
    std::vector<double> velocity;
    std::vector<double> position;

    std::vector<Particle*> neighbors;
    
    std::vector<double> personalBestPosition;
    double personalBestFitness = std::numeric_limits<double>::infinity();
    std::vector<double> localBestPosition;
    double localBestFitness = std::numeric_limits<double>::infinity();

    Particle() = default;
    explicit Particle(std::size_t vectorSize) : velocity(vectorSize), position(vectorSize) {}

    friend std::ostream& operator<<(std::ostream& os, const Particle& p) {
        os << "Particle(fitness=" << p.personalBestFitness << ", v=[";
        for (size_t i = 0; i < p.position.size(); ++i) {
            os << p.position[i];
            if (i + 1 < p.position.size()) os << ", ";
        }
        os << "])";
        return os;
    }
};

enum Neighborhood {
    GLOBAL,
    RING
};

class PSO {
    private:
        std::size_t populationSize;
        Neighborhood neighborhood;
        double cognitiveComponent;
        double socialComponent;
        double inertia;
        std::size_t costCalculations;

        ISystem* system;
        std::size_t vectorSize;
        std::mt19937 rng;
        std::uniform_real_distribution<double> initPosDist;
        std::uniform_real_distribution<double> initVelDist;
        std::uniform_real_distribution<double> probDist;

        std::vector<Particle> population;
        std::vector<double> bestPosition;
        double bestFitness;

    public:
        PSO(
            std::size_t populationSize,
            Neighborhood neighborhood,
            double cognitiveComponent,
            double socialComponent,
            double inertia,
            std::size_t costCalculations,
            double initPopBoundLower,
            double initPopBoundUpper,
            ISystem* system
        );

        Particle getBest() const;

        void initialize();

        void train(bool printNewBest = false);
};