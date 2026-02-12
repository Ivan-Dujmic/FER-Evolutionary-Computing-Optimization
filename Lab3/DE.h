#pragma once
#include "ISystem.h"
#include <random>
#include <limits>
#include <iostream>

struct Unit {
    std::vector<double> v;
    double fitness = std::numeric_limits<double>::infinity();

    Unit() = default;
    explicit Unit(std::size_t vectorSize) : v(vectorSize) {}

    bool operator==(const Unit& other) const {
        return fitness == other.fitness && v == other.v;
    }

    bool operator!=(const Unit& other) const {
        return !(*this == other);
    }

    friend std::ostream& operator<<(std::ostream& os, const Unit& u) {
        os << "Unit(fitness=" << u.fitness << ", v=[";
        for (size_t i = 0; i < u.v.size(); ++i) {
            os << u.v[i];
            if (i + 1 < u.v.size()) os << ", ";
        }
        os << "])";
        return os;
    }
};

enum BaseChoice {
    RAND,
    BEST,
    TARGET_TO_BEST
};

enum CrossChoice {
    EXP,
    BIN
};

class DE {
    private:
        BaseChoice baseChoice;
        std::size_t nDifferenceVectors; // Vectors for creating the mutant (will choose 2 * n units)
        CrossChoice crossChoice;
        double mutantProbability; // CR
        double vectorScaler; // F
        std::size_t populationSize;
        std::size_t costCalculations; // Number of cost calculations before the algorithm stops

        std::mt19937 rng;
        std::uniform_real_distribution<double> initPopDist; // Distribution for the individual values of unit vectors between given bounds
        std::uniform_int_distribution<std::size_t> populationDist; // [0, populationSize - 1]
        std::uniform_real_distribution<double> probDist; // [0, 1]
        std::uniform_int_distribution<std::size_t> coefDist; // [0, vectorSize - 1]

        ISystem* system;
        std::size_t vectorSize; // Size of the Unit.v vectors (number of coefficients)

        std::vector<Unit> population;
        Unit best;

    public:
        DE(
            BaseChoice baseChoice,
            std::size_t nDifferenceVectors,
            CrossChoice crossChoice,
            double mutantProbability,
            double vectorScaler,
            std::size_t populationSize,
            std::size_t costCalculations,
            double initPopBoundLower,
            double initPopBoundUpper,
            ISystem* system
        );

        Unit getBest() const;

        void initialize();

        void train(bool printNewBest = false);
};