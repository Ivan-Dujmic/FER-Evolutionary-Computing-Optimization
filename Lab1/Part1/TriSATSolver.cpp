#include "BitVector.h"
#include "BitVectorNGenerator.h"
#include "Clause.h"
#include "IOptAlgorithm.h"
#include "MutableBitVector.h"
#include "SATFormula.h"
#include "SATFormulaStats.h"

#include "OABruteForce.h"
#include "OAGreedyHillClimb.h"
#include "OAStatsSearch.h"
#include "OAGSAT.h"
#include "OARandomWalkSAT.h"
#include "OAPerturbatingILS.h"

#include <fstream>
#include <iostream>
#include <unordered_map>

std::vector<std::string> split(const std::string& str, char delimiter = ' ') {
    std::vector<std::string> result;
    std::istringstream ss(str);
    std::string token;

    while (std::getline(ss, token, delimiter)) {
        if (token[0] == ' ') token.erase(0, 1);
        if (token[token.size() - 1] == ' ') token.erase(token.size() - 1, 1);
        result.push_back(token);
    }

    return result;
}

std::vector<int> toIntVector(const std::vector<std::string>& strVector) {
    std::vector<int> intVector;
    intVector.reserve(strVector.size());

    for (const auto& s : strVector) {
        if (s != "") intVector.push_back(std::stoi(s));
    }

    return intVector;
}

SATFormula parse(std::string fileName) {
    std::ifstream file(fileName);

    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << fileName << '\n';
        exit(1);
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line[0] == 'c') continue;
        if (line[0] == 'p') {
            std::vector<std::string> parts = split(line);
            std::vector<Clause> clauses;

            while (std::getline(file, line)) {
                if (line[0] == 'c') continue;
                if (line[0] == '%') break;

                std::vector<std::string> clause_parts = split(line, ' ');
                clause_parts.pop_back();
                clauses.push_back(Clause(toIntVector(clause_parts)));
            }

            return SATFormula(std::stoi(parts[2]), clauses);
        }
    }

    exit(1);
}

std::pair<bool, std::string> checkOption(char* argv[], int argc, std::string option, bool hasArgument) {
    for (int i = 3 ; i < argc ; i++) {
        std::string arg = argv[i];
        if (arg == option) {
            if (hasArgument) {
                if (i == argc - 1) {
                    std::cerr << "Missing option argument\n";
                    exit(1);
                }

                std::string opt_arg = argv[i+1];
                return std::make_pair(true, opt_arg);
            } else {
                return std::make_pair(true, "");
            }
        }
    }

    return std::make_pair(false, "");
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <algorithm> <problem> [options]\n";
        return 1;
    }

    std::string algorithm = argv[1];
    std::string problem = argv[2];

    SATFormula formula = parse(problem);

    std::unordered_map<std::string, int> algorithms = {
        {"BruteForce", 0},  // -printAll
        {"GreedyHillClimb", 1}, // -maxIter #
        {"StatsSearch", 2}, // -maxIter # -numberOfBest # -percentageConstantUp # -percentageConstantDown # -percentageUnitAmount #
        {"GSAT", 3},    // -maxTries # -maxFlips #
        {"RandomWalkSAT", 4},   // -maxTries # -maxFlips # -p #
        {"PerturbatingILS", 5}  // -maxIter # -toChange #
    };

    std::optional<BitVector> solution = {};

    switch (algorithms[algorithm]) {
        case 0: {
            bool printAll = checkOption(argv, argc, "-printAll", false).first;
            BruteForce algorithm = BruteForce(formula, printAll);
            std::optional<BitVector> initial = {};
            solution = algorithm.solve(initial);

            if (solution.has_value()) {
                if (!printAll) std::cout << solution.value().toString() << '\n';
            } else {
                std::cout << "No solution found\n";
            }

            break;
        }

        case 1: {
            int maxIterations = 100000;
            std::pair<bool, std::string> option = checkOption(argv, argc, "-maxIter", true);
            if (option.first) maxIterations = std::stoi(option.second);

            GreedyHillClimb algorithm = GreedyHillClimb(formula, maxIterations);
            std::optional<BitVector> initial = {};
            solution = algorithm.solve(initial);

            if (solution.has_value()) std::cout << solution.value().toString() << '\n';
            else std::cout << "No solution found\n";

            break;
        }

        case 2: {
            int maxIterations = 100000;
            int numberOfBest = 2;
            double percentageConstantUp = 0.01;
            double percentageConstantDown = 0.1;
            double percentageUnitAmount = 50.0;

            std::pair<bool, std::string> option;

            option = checkOption(argv, argc, "-maxIter", true);
            if (option.first) maxIterations = std::stoi(option.second);

            option = checkOption(argv, argc, "-numberOfBest", true);
            if (option.first) numberOfBest = std::stoi(option.second);

            option = checkOption(argv, argc, "-percentageConstantUp", true);
            if (option.first) percentageConstantUp = std::stod(option.second);

            option = checkOption(argv, argc, "-percentageConstantDown", true);
            if (option.first) percentageConstantDown = std::stod(option.second);

            option = checkOption(argv, argc, "-percentageUnitAmount", true);
            if (option.first) percentageUnitAmount = std::stod(option.second);

            StatsSearch algorithm = StatsSearch(
                formula,
                maxIterations,
                numberOfBest,
                percentageConstantUp,
                percentageConstantDown,
                percentageUnitAmount
            );

            std::optional<BitVector> initial = {};
            solution = algorithm.solve(initial);

            if (solution.has_value())
                std::cout << solution.value().toString() << '\n';
            else
                std::cout << "No solution found\n";

            break;
        }

        case 3: {
            int maxTries = 100;
            int maxFlips = 1000;

            std::pair<bool, std::string> option;

            option = checkOption(argv, argc, "-maxTries", true);
            if (option.first) maxTries = std::stoi(option.second);

            option = checkOption(argv, argc, "-maxFlips", true);
            if (option.first) maxFlips = std::stoi(option.second);

            GSAT algorithm = GSAT(formula, maxTries, maxFlips);

            std::optional<BitVector> initial = {};
            solution = algorithm.solve(initial);

            if (solution.has_value())
                std::cout << solution.value().toString() << '\n';
            else
                std::cout << "No solution found\n";

            break;
        }

        case 4: {
            int maxTries = 100;
            int maxFlips = 1000;
            double p = 0.1;

            std::pair<bool, std::string> option;

            option = checkOption(argv, argc, "-maxTries", true);
            if (option.first) maxTries = std::stoi(option.second);

            option = checkOption(argv, argc, "-maxFlips", true);
            if (option.first) maxFlips = std::stoi(option.second);

            option = checkOption(argv, argc, "-p", true);
            if (option.first) p = std::stod(option.second);

            RandomWalkSAT algorithm = RandomWalkSAT(formula, maxTries, maxFlips, p);

            std::optional<BitVector> initial = {};
            solution = algorithm.solve(initial);

            if (solution.has_value())
                std::cout << solution.value().toString() << '\n';
            else
                std::cout << "No solution found\n";

            break;
        }

        case 5: {
            int maxIterations = 100000;
            double toChange = 0.1;

            std::pair<bool, std::string> option;

            option = checkOption(argv, argc, "-maxIter", true);
            if (option.first) maxIterations = std::stoi(option.second);

            option = checkOption(argv, argc, "-toChange", true);
            if (option.first) toChange = std::stod(option.second);

            PerturbatingILS algorithm = PerturbatingILS(formula, maxIterations, toChange);
            std::optional<BitVector> initial = {};
            solution = algorithm.solve(initial);

            if (solution.has_value()) std::cout << solution.value().toString() << '\n';
            else std::cout << "No solution found\n";

            break;
        }

        default: {
            std::cerr << "Unknown algorithm: " << algorithm << '\n';
            return 1;
        }
    }
}