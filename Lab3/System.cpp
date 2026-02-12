#include "System.h"
#include <cmath>

// The vector argument takes x1, x2, x3, x4, x5 and the convertion to the Sample struct will be done in the constructor
System::System(std::size_t vectorSize, const std::vector<std::vector<double>>& samples) :
    ISystem(vectorSize),
    samples([&] {
        std::vector<Sample> tmp;
        tmp.reserve(samples.size());

        for (const auto& sample : samples) {
            tmp.push_back({
                sample[0],
                sample[0] * sample[0] * sample[0],
                sample[1],
                sample[2],
                sample[3],
                sample[4] * sample[4],
                sample[5]
            });
        }

        return tmp;
    }()) {}

// Returns the mean squares error
double System::getOptimizationParameter(const std::vector<double>& coef) const {
    double a = coef[0], b = coef[1], c = coef[2], d = coef[3], e = coef[4], f = coef[5];
    double error = 0.0;
    for (const Sample& sample : samples) {
        double x1 = sample.x1, x1c = sample.x1Cubed, x2 = sample.x2, x3 = sample.x3, x4 = sample.x4, x5s = sample.x5Squared, y = sample.y;
        double output = a * x1 + b * x1c * x2 + c * std::exp(d * x3) * (1 + std::cos(e * x4)) + f * x4 * x5s;
        double diff = output - y;
        error += (diff * diff);
    }
    return error;
} 