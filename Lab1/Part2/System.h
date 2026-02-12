#pragma once
#include "ISystem.h"

class System : public ISystem {
    private:
        std::vector<std::vector<double>> samples;

    public:
        System(std::vector<std::vector<double>> samples);

        // Returns the mean squares error
        double getOptimizationParameter(std::vector<double> coef);
};