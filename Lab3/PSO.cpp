#include "PSO.h"
#include <iostream>

PSO::PSO(
    std::size_t populationSize,
    Neighborhood neighborhood,
    double cognitiveComponent,
    double socialComponent,
    double inertia,
    std::size_t costCalculations,
    double initPopBoundLower,
    double initPopBoundUpper,
    ISystem* system
) :
populationSize(populationSize),
neighborhood(neighborhood),
cognitiveComponent(cognitiveComponent),
socialComponent(socialComponent),
inertia(inertia),
costCalculations(costCalculations),
system(system),
vectorSize(system->getVectorSize()),
rng(std::random_device{}()),
initPosDist(initPopBoundLower, initPopBoundUpper),
initVelDist(0 - 0.15 * (initPopBoundUpper - initPopBoundLower), 0 + 0.15 * (initPopBoundUpper - initPopBoundLower)),
probDist(0, 1),
population(populationSize, Particle(system->getVectorSize())),
bestFitness(std::numeric_limits<double>::infinity())
{}

Particle PSO::getBest() const {
    Particle best = Particle(this->system->getVectorSize());
    best.localBestFitness = this->bestFitness;
    best.localBestPosition = this->bestPosition;
    return best;
}

void PSO::initialize() {
    for (Particle& p : this->population) {
        for (double& x : p.position) {
            x = this->initPosDist(this->rng);
        }

        for (double& v : p.velocity) {
            v = this->initVelDist(this->rng);
        }

        p.personalBestPosition = p.position;
        p.personalBestFitness = this->system->getOptimizationParameter(p.position);
    }

    for (Particle& p : this->population) {
        if (p.personalBestFitness < this->bestFitness) {
            this->bestFitness = p.personalBestFitness;
            this->bestPosition = p.personalBestPosition;
        }
    }

    switch (this->neighborhood) {
        case Neighborhood::GLOBAL:
            break;

        case Neighborhood::RING:
            for (std::size_t i = 1 ; i < this->populationSize - 1 ; i++) {
                this->population[i].neighbors.push_back(&(this->population[i-1]));
                this->population[i].neighbors.push_back(&(this->population[i+1]));
            }
            this->population[0].neighbors.push_back(&(this->population[1]));
            this->population[0].neighbors.push_back(&(this->population[this->populationSize-1]));
            this->population[this->populationSize].neighbors.push_back(&(this->population[0]));
            this->population[this->populationSize].neighbors.push_back(&(this->population[this->populationSize-2]));

            for (Particle& p : this->population) {
                p.localBestFitness = p.personalBestFitness;
                p.localBestPosition = p.personalBestPosition;

                for (Particle* n : p.neighbors) {
                    if (n->personalBestFitness < p.localBestFitness) {
                        p.localBestFitness = n->personalBestFitness;
                        p.localBestPosition = n->personalBestPosition;
                    }
                }
            }

            break;
    }
}

void PSO::train(bool printNewBest) {
    std::size_t calculations = 0;
    while (calculations < this->costCalculations) {
        for (Particle& p : this->population) {
            std::vector<double> localBest;
            if (this->neighborhood == Neighborhood::GLOBAL) localBest = this->bestPosition;
            else localBest = p.localBestPosition;

            for (std::size_t i = 0 ; i < this->vectorSize ; i++) {
                p.velocity[i] = p.velocity[i] * this->inertia + 
                                this->cognitiveComponent * this->probDist(this->rng) * (p.personalBestPosition[i] - p.position[i]) +
                                this->socialComponent * this->probDist(this->rng) * (localBest[i] - p.position[i]);
            }

            for (std::size_t i = 0 ; i < this->vectorSize ; i++) {
                p.position[i] += p.velocity[i];
            }

            double fitness = this->system->getOptimizationParameter(p.position);
            calculations++;
            
            if (fitness < p.personalBestFitness) {
                p.personalBestFitness = fitness;
                p.personalBestPosition = p.position;

                if (fitness < this->bestFitness) {
                    this->bestFitness = fitness;
                    this->bestPosition = p.position;
                    if (printNewBest) {
                        std::cout << "Calculations: " << calculations << " | New best: " << p << '\n';
                    }
                }

                if (this->neighborhood != Neighborhood::GLOBAL) {
                    if (fitness < p.localBestFitness) {
                        p.localBestFitness = fitness;
                        p.localBestPosition = p.position;       
                    }
    
                    for (Particle* n : p.neighbors) {
                        if (fitness < n->localBestFitness) {
                            n->localBestFitness = fitness;
                            n->localBestPosition = p.position;
                        }
                    }
                }
            }
        }
    }
}