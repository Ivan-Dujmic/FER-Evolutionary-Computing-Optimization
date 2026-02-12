#include "DE.h"
#include <unordered_set>
#include <iostream>

std::vector<double> subtractVectors(const std::vector<double>& v1, const std::vector<double>& v2) {
    std::vector<double> result;
    result.reserve(v1.size());

    for (std::size_t i = 0 ; i < v1.size() ; i++) {
        result.push_back(v1[i] - v2[i]);
    }

    return result;
}

void scaleVectorAndAdd(std::vector<double>& base, const std::vector<double>& v, double scaler) {
    for (std::size_t i = 0 ; i < base.size() ; i++) {
        base[i] += (scaler * v[i]);
    }
}

DE::DE(
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
) :
    baseChoice(baseChoice),
    nDifferenceVectors(nDifferenceVectors),
    crossChoice(crossChoice),
    mutantProbability(mutantProbability),
    vectorScaler(vectorScaler),
    populationSize(populationSize),
    costCalculations(costCalculations),
    rng(std::random_device{}()),
    initPopDist(initPopBoundLower, initPopBoundUpper),
    populationDist(0, populationSize - 1),
    probDist(0, 1),
    coefDist(0, system->getVectorSize() - 1),
    system(system),
    vectorSize(system->getVectorSize()),
    population(populationSize, Unit(vectorSize))
    {}

Unit DE::getBest() const {
    return this->best;
}

void DE::initialize() {
    for (auto& unit : this->population) {
        for (double& x : unit.v) {
            x = this->initPopDist(this->rng);
        }
        unit.fitness = this->system->getOptimizationParameter(unit.v);
        if (unit.fitness < this->best.fitness) {
            this->best = unit;
        }
    }
}

void DE::train(bool printNewBest) {
    std::size_t calculations = 0;
    while (calculations < this->costCalculations) {
        std::vector<Unit> newPopulation;
        newPopulation.reserve(this->populationSize);

        Unit newBest = this->best;

        for (std::size_t i = 0 ; i < this->populationSize ; i++) {
            Unit& orig = this->population[i];
            std::vector<double> mutant;

            switch (this->baseChoice) {
                case BaseChoice::RAND:
                    mutant = this->population[this->populationDist(rng)].v;
                    break;

                case BaseChoice::BEST:
                    mutant = this->best.v;
                    break;

                case BaseChoice::TARGET_TO_BEST:
                    mutant = orig.v;
                    std::vector<double> diff = subtractVectors(this->best.v, orig.v);
                    scaleVectorAndAdd(mutant, diff, this->vectorScaler);
                    break;
            }

            // nDifferenceVectors will usually be small so this is a valid approach
            // If it were large then we would opt for a population (indices) shuffle
            std::unordered_set<std::size_t> chosenIndices;
            while (chosenIndices.size() < 2 * this->nDifferenceVectors) {
                std::size_t index = this->populationDist(this->rng);
                if (index != i) {
                    chosenIndices.insert(index);
                }
            }

            auto it = chosenIndices.begin();
            std::vector<double> sum(this->vectorSize);
            for (std::size_t j = 0 ; j < 2 * this->nDifferenceVectors ; j+=2) {
                std::vector<double> diff = subtractVectors(this->population[*it].v, this->population[*(++it)].v);
                
                for (std::size_t k = 0 ; k < sum.size() ; k++) {
                    sum[k] += diff[k];
                }
            }

            scaleVectorAndAdd(mutant, sum, this->vectorScaler);

            Unit trial = orig;

            switch (this->crossChoice) {
                case CrossChoice::EXP: {
                    std::size_t start = this->coefDist(this->rng);
                    trial.v[start] = mutant[start]; // One is guaranteed to be copied from the mutant

                    for (std::size_t j = start + 1 ; j != start ; j++) {
                        if (j == trial.v.size()) {
                            j = 0;
                            if (start == 0) break;
                        }
                        
                        if (this->probDist(this->rng) < this->mutantProbability) {
                            trial.v[j] = mutant[j];
                        } else break;
                    }
                    break;
                }

                case CrossChoice::BIN: {
                    std::size_t guaranteed = this->coefDist(this->rng);
                    trial.v[guaranteed] = mutant[guaranteed]; // One is guaranteed to be copied from the mutant
                    for (std::size_t j = 0 ; j < trial.v.size() ; j++) {
                        if (this->probDist(this->rng) < this->mutantProbability) {
                            trial.v[j] = mutant[j];
                        }
                    }
                    break;
                }
            }

            trial.fitness = this->system->getOptimizationParameter(trial.v);
            calculations++;
            if (trial.fitness <= orig.fitness) {
                newPopulation.push_back(trial);
                if (trial.fitness <= newBest.fitness) {
                    newBest = trial;
                }
            } else {
                newPopulation.push_back(orig);
            }
        }

        if (printNewBest && newBest != this->best) {
            std::cout << "Calculations: " << calculations << " | New best: " << newBest << '\n';
        }

        this->population = newPopulation;
        this->best = newBest;
    }
}