#include "SimulatedAnnealing.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <numbers>
#include <sstream>
#include <fstream>
#include "System.h"

std::pair<bool, std::string> checkOption(char* argv[], int argc, std::string option) {
    for (int i = 2 ; i < argc ; i++) {
        std::string arg = argv[i];
        if (arg == option) {
            if (i == argc - 1) {
                std::cerr << "Missing option argument\n";
                exit(1);
            }

            std::string opt_arg = argv[i+1];
            return std::make_pair(true, opt_arg);
        }
    }

    return std::make_pair(false, "");
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename> [options]\n";
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << argv[1] << '\n';
        return 1;
    }

    std::vector<std::vector<double>> data;
    std::string line;

    while (std::getline(file, line)) {
        std::vector<double> row;
        std::string cleaned;

        for (char c : line) {
            if (c != '[' && c != ']' && c != ',') cleaned += c;
        }

        std::stringstream ss(cleaned);
        double val;
        while (ss >> val) {
            row.push_back(val);
        }
        
        if (!row.empty()) {
            data.push_back(row);
        }
    }

    file.close();

    System system = System(data);

    // bool maximize, int numOfVariables, int (*function)(std::vector<double>), int maxIterations, double initialTemp, double alpha);

    int maxIterations = 5000;
    int equilibrium = 50;
    double initialTemp = 100.0;
    double alpha = 0.95;
    double neighbourMaxChange = 0.5;

    std::pair<bool, std::string> option;

    option = checkOption(argv, argc, "-maxIter");
    if (option.first) maxIterations = std::stoi(option.second);

    option = checkOption(argv, argc, "-equilibrium");
    if (option.first) equilibrium = std::stoi(option.second);

    option = checkOption(argv, argc, "-T");
    if (option.first) initialTemp = std::stod(option.second);

    option = checkOption(argv, argc, "-alpha");
    if (option.first) alpha = std::stod(option.second);

    option = checkOption(argv, argc, "-nbmc");
    if (option.first) neighbourMaxChange = std::stod(option.second);

    std::vector<double> solution = SimulatedAnnealing(false, 6, &system, maxIterations, equilibrium, initialTemp, alpha, neighbourMaxChange).run();

    for (int i = 0 ; i < 6 ; i++) {
        std::cout << solution[i] << ' ';
    }
    std::cout << solution[6] << '\n';
}