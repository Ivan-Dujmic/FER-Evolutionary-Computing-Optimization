#include "Problem1.h"
#include "Problem2.h"
#include "NSGA2.h"
#include <sstream>
#include <fstream>

std::vector<std::string> split(const std::string& input, char sep) {
    std::vector<std::string> result;
    std::stringstream ss(input);
    std::string token;

    while (std::getline(ss, token, sep)) {
        if (!token.empty() && token.front() == ' ') token.erase(token.begin());
        if (!token.empty() && token.back() == ' ') token.pop_back();
        result.push_back(token);
    }

    return result;
}

int main() {
    std::ifstream configs("config.txt");
    if (!configs.is_open()) {
        std::cerr << "Error opening file: \"config.txt\"\n";
        return 1;
    }

    std::string line;
    std::vector<std::string> parts;

    std::getline(configs, line);
    parts = split(line, '=');

    MOOPProblem* problem;
    if (parts[1] == "1") {
        std::pair problemBoundaryPair(-5, 5);
        std::vector<std::pair<double, double>> problemBoundaries = {problemBoundaryPair, problemBoundaryPair, problemBoundaryPair, problemBoundaryPair};
        problem = new Problem1(problemBoundaries);
    } else if (parts[1] == "2") {
        std::vector<std::pair<double, double>> problemBoundaries = {std::make_pair(0.1, 1), std::make_pair(0, 5)};
        problem = new Problem2(problemBoundaries);
    } else {
        std::cerr << "Problem " << parts[1] << " is not defined\n";
        return 2;
    }

    std::getline(configs, line);
    parts = split(line, '=');
    std::size_t populationSize = std::stoi(parts[1]);

    std::getline(configs, line);
    parts = split(line, '=');
    double mutationChance = std::stod(parts[1]);

    std::getline(configs, line);
    parts = split(line, '=');
    double mutationStrength = std::stod(parts[1]);

    std::getline(configs, line);
    parts = split(line, '=');
    std::size_t iterations = std::stoi(parts[1]);

    configs.close();

    NSGA2 nsga2(populationSize, mutationChance, mutationStrength, iterations, problem);

    nsga2.initializePopulation();
    nsga2.train();
    nsga2.writeResults("results.txt");

    return 0;
}