#pragma once
#include <vector>

class ISystem {
    public:
        // Returns the optimization parameter
        virtual double getOptimizationParameter(std::vector<double> coef) = 0;
};