// g++ main.cpp DE.cpp System.cpp -o main -g -O3

#include "DE.h"
#include "PSO.h"
#include "System.h"
#include <fstream>
#include <sstream>

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
    std::ifstream dataFile("data.txt");
    if (!dataFile.is_open()) {
        std::cerr << "Error opening file: \"data.txt\"";
        return 1;
    }

    std::vector<std::vector<double>> data;
    std::string line;

    while (std::getline(dataFile, line)) {
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

    dataFile.close();

    System system = System(6, data);

    std::ifstream configFile("config.txt");
    if (!configFile.is_open()) {
        std::cerr << "Error opening file: \"config.txt\"";
        return 1;
    }

    std::string algorithm;
    BaseChoice baseChoice;
    std::size_t nDifferenceVectors;
    CrossChoice crossChoice;
    double mutantProbability;
    double vectorScaler;
    std::size_t populationSize;
    std::size_t costCalculations;
    double initPopBoundLower;
    double initPopBoundUpper;
    Neighborhood neighborhood;
    double cognitiveComponent;
    double socialComponent;
    double inertia;

    std::getline(configFile, line);
    std::vector<std::string> parts = split(line, '/');
    if (parts[0] == "DE") {
        algorithm = "DE";
        if (parts[1] == "RAND") baseChoice = RAND;
        else if (parts[1] == "BEST") baseChoice = BEST;
        else if (parts[1] == "TARGET_TO_BEST") baseChoice = TARGET_TO_BEST;
        else {
            std::cerr << "Invalid baseChoice: " << parts[1] << '\n';
            return 1;
        }
    
        nDifferenceVectors = std::stoi(parts[2]);
    
        if (parts[3] == "EXP") crossChoice = EXP;
        else if (parts[3] == "BIN") crossChoice = BIN;
        else {
            std::cerr << "Invalid crossChoice: " << parts[3] << '\n';
            return 1;
        }
    
        std::getline(configFile, line);
        parts = split(line, '=');
        mutantProbability = std::stod(parts[1]);
    
        std::getline(configFile, line);
        parts = split(line, '=');
        vectorScaler = std::stod(parts[1]);
    } else if (parts[0] == "PSO") {
        algorithm = "PSO";
        if (parts[1] == "GLOBAL") neighborhood = GLOBAL;
        else if (parts[1] == "RING") neighborhood = RING;
        else {
            std::cerr << "Invalid neighborhood: " << parts[1] << '\n';
            return 1;
        }

        std::getline(configFile, line);
        parts = split(line, '=');
        cognitiveComponent = std::stod(parts[1]);

        std::getline(configFile, line);
        parts = split(line, '=');
        socialComponent = std::stod(parts[1]);

        std::getline(configFile, line);
        parts = split(line, '=');
        inertia = std::stod(parts[1]);
    }
    

    std::getline(configFile, line);
    parts = split(line, '=');
    populationSize = std::stoi(parts[1]);

    std::getline(configFile, line);
    parts = split(line, '=');
    costCalculations = std::stoi(parts[1]);

    std::getline(configFile, line);
    parts = split(line, '=');
    initPopBoundLower = std::stod(parts[1]);

    std::getline(configFile, line);
    parts = split(line, '=');
    initPopBoundUpper = std::stod(parts[1]);

    if (algorithm == "DE") {
        DE de = DE(baseChoice, nDifferenceVectors, crossChoice, mutantProbability, vectorScaler, populationSize, costCalculations, initPopBoundLower, initPopBoundUpper, &system);
        de.initialize();
        std::cout << "Initial best: " << de.getBest() << '\n';
        de.train(true);
        std::cout << "Final best: " << de.getBest() << '\n';
    } else if (algorithm == "PSO") {
        PSO pso = PSO(populationSize, neighborhood, cognitiveComponent, socialComponent, inertia, costCalculations, initPopBoundLower, initPopBoundUpper, &system);
        pso.initialize();
        std::cout << "Initial best: " << pso.getBest() << '\n';
        pso.train(true);
        std::cout << "Final best: " << pso.getBest() << '\n';
    }

    configFile.close();

    return 0;
}