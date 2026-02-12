#include "System.h"
#include <cmath>

System::System(std::vector<std::vector<double>> samples) : samples(samples) {}

// Returns the mean squares error
double System::getOptimizationParameter(std::vector<double> coef) {
    double error = 0.0;
    for (std::vector<double> sample : samples) {
        double output = coef[0] * sample[0] + coef[1] * std::pow(sample[0], 3) * sample[1] + coef[2] * std::pow(std::numbers::e, coef[3] * sample[2]) * (1 + std::cos(coef[4] * sample[3])) + coef[5] * sample[3] * std::pow(sample[4], 2);
        error += std::pow(output - sample[5], 2);
    }
    return std::sqrt(error);
}