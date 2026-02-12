#pragma once
#include <vector>
#include <cstddef>

class ISystem {
    protected:
        std::size_t vectorSize;

    public:
        explicit ISystem(std::size_t vectorSize) : vectorSize(vectorSize) {}
        virtual ~ISystem() = default;
        std::size_t getVectorSize() const {return vectorSize;}
        virtual double getOptimizationParameter(const std::vector<double>& coef) const = 0;
};