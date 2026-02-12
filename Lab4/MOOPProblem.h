#pragma once
#include <vector>
#include <utility>

class MOOPProblem {
    protected:
        std::size_t dimensions;
        std::vector<std::pair<double, double>> boundaries;

    public:
        MOOPProblem(std::size_t dimensions, std::vector<std::pair<double, double>> boundaries) : dimensions(dimensions), boundaries(std::move(boundaries)) {}

        int getDimensions() const {
            return this->dimensions;
        }

        int getNumberOfObjectives() const {
            return this->boundaries.size();
        }

        const std::vector<std::pair<double, double>>& getBoundaries() const {
            return this->boundaries;
        }

        std::pair<double, double> getBoundary(std::size_t index) const {
            return this->boundaries[index];
        }

        virtual void evaluateSolution(const std::vector<double>& solution, std::vector<double>& fitness) const = 0;
};