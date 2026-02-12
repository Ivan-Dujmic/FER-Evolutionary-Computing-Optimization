#pragma once
#include "ISystem.h"

struct Sample {
    double x1;
    double x1Cubed;
    double x2;
    double x3;
    double x4;
    double x5Squared;
    double y;
};

class System : public ISystem {
    private:
        const std::vector<Sample> samples;

    public:
        // The vector argument takes x1, x2, x3, x4, x5 and the convertion to the Sample struct will be done in the constructor
        System(std::size_t vectorSize, const std::vector<std::vector<double>>& samples);

        // Returns the mean squares error
        double getOptimizationParameter(const std::vector<double>& coef) const override;
};