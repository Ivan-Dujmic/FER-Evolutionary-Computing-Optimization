#include "GPTree.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <sstream>
#include <algorithm>

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

struct Context {
    double x1;  // input
    double x2;  // input
    double y;   // expected output

    double valueMin;    // min value for constants
    double valueMax;    // max value for constants

    mutable std::mt19937 rng;
};

double myAdd(std::string* symbol, const std::vector<std::unique_ptr<GPNode<double, Context>>>& children, const Context& context) {
    return children[0]->evaluate(context) + children[1]->evaluate(context);
}

double mySub(std::string* symbol, const std::vector<std::unique_ptr<GPNode<double, Context>>>& children, const Context& context) {
    return children[0]->evaluate(context) - children[1]->evaluate(context);
}

double myMul(std::string* symbol, const std::vector<std::unique_ptr<GPNode<double, Context>>>& children, const Context& context) {
    return children[0]->evaluate(context) * children[1]->evaluate(context);
}

double myDiv(std::string* symbol, const std::vector<std::unique_ptr<GPNode<double, Context>>>& children, const Context& context) {
    double child1 = children[1]->evaluate(context);
    if (child1 == 0) return 1.0;
    return children[0]->evaluate(context) / child1;
}

double mySin(std::string* symbol, const std::vector<std::unique_ptr<GPNode<double, Context>>>& children, const Context& context) {
    return std::sin(children[0]->evaluate(context));
}

double myCos(std::string* symbol, const std::vector<std::unique_ptr<GPNode<double, Context>>>& children, const Context& context) {
    return std::cos(children[0]->evaluate(context));
}

double mySqrt(std::string* symbol, const std::vector<std::unique_ptr<GPNode<double, Context>>>& children, const Context& context) {
    double child1 = children[0]->evaluate(context);
    if (child1 < 0) return 1.0;
    return std::sqrt(child1);
}

double myLog(std::string* symbol, const std::vector<std::unique_ptr<GPNode<double, Context>>>& children, const Context& context) {
    double child1 = children[0]->evaluate(context);
    if (child1 <= 0) return 1.0;
    return std::log10(child1);
}

double myExp(std::string* symbol, const std::vector<std::unique_ptr<GPNode<double, Context>>>& children, const Context& context) {
    return std::exp(children[0]->evaluate(context));
}

double x1(std::string* symbol, const std::vector<std::unique_ptr<GPNode<double, Context>>>& children, const Context& context) {
    return context.x1;
}

double x2(std::string* symbol, const std::vector<std::unique_ptr<GPNode<double, Context>>>& children, const Context& context) {
    return context.x2;
}

double value(std::string* symbol, const std::vector<std::unique_ptr<GPNode<double, Context>>>& children, const Context& context) {
    if (*symbol == "") {
        std::uniform_real_distribution<double> dist(context.valueMin, context.valueMax);
        *symbol = std::to_string(dist(context.rng));
    }
    return std::stod(*symbol);
}

class Penalty : public IPenalty<double, Context> {
    public:
        double calculate(std::vector<Context>& contexts, GPNode<double, Context>& tree) {
            double penalty = 0;

            for (const Context& context : contexts) {
                double diff = tree.evaluate(context) - context.y;
                penalty += (diff * diff);
            }

            return std::sqrt(penalty);
        } 
};

int main() {
    std::ifstream parameters("parameters.txt");
    if (!parameters.is_open()) {
        std::cerr << "Failed to open parameters.txt\n";
        exit(1);
    }

    std::unordered_map<std::string, GPOperator<double, Context>> operatorsMap = {
        {"add", {"+", myAdd, 2}},
        {"sub", {"-", mySub, 2}},
        {"mul", {"*", myMul, 2}},
        {"div", {"/", myDiv, 2}},
        {"sin", {"sin", mySin, 1}},
        {"cos", {"cos", myCos, 1}},
        {"sqrt", {"sqrt", mySqrt, 1}},
        {"log", {"log", myLog, 1}},
        {"exp", {"exp", myExp, 1}},

        {"x1", {"x1", x1, 0}},
        {"x2", {"x2", x2, 0}},
        {"val", {"", value, 0}}
    };

    std::string line;
    std::vector<std::string> parts;

    // Operators
    std::getline(parameters, line);
    parts = split(line, ' ');
    std::vector<GPOperator<double, Context>> operators;
    for (std::size_t i = 1 ; i < parts.size() ; i++) {
        operators.push_back(operatorsMap[parts[i]]);
    }

    // Constants range
    std::getline(parameters, line);
    parts = split(line, ' ');
    double minValue = 0;
    double maxValue = 0;
    if (parts.size() == 3) {
        operators.push_back(operatorsMap["val"]);
        minValue = std::stod(parts[1]);
        maxValue = std::stod(parts[2]);
    }

    // Population size
    std::getline(parameters, line);
    parts = split(line, ' ');
    std::size_t populationSize = std::stoi(parts[1]);

    // Tournamet size
    std::getline(parameters, line);
    parts = split(line, ' ');
    std::size_t tournamentSize = std::stoi(parts[1]);

    // Cost evaluations
    std::getline(parameters, line);
    parts = split(line, ' ');
    std::size_t costEvaluations = std::stoi(parts[1]);
    
    // Elitism
    std::getline(parameters, line);
    parts = split(line, ' ');
    std::size_t elitism = std::stoi(parts[1]);

    // Clone probability
    std::getline(parameters, line);
    parts = split(line, ' ');
    double pClone = std::stod(parts[1]);

    // Mutate probability
    std::getline(parameters, line);
    parts = split(line, ' ');
    double pMutate = std::stod(parts[1]);

    // Cross probability
    std::getline(parameters, line);
    parts = split(line, ' ');
    double pCross = std::stod(parts[1]);

    // Max tree depth
    std::getline(parameters, line);
    parts = split(line, ' ');
    std::size_t maxDepth = std::stoi(parts[1]);

    // Max tree nodes
    std::getline(parameters, line);
    parts = split(line, ' ');
    std::size_t maxNodes = std::stoi(parts[1]);

    // Problem file in
    std::getline(parameters, line);
    parts = split(line, ' ');
    std::string problemPath = parts[1];

    std::vector<Context> contexts;
    std::ifstream problem(problemPath);
    std::size_t inputSize = 0;
    while (std::getline(problem, line)) {
        parts = split(line, '\t');
        if (inputSize == 0) {
            inputSize = parts.size() - 1;
            operators.push_back(operatorsMap["x1"]);
            if (inputSize > 1) operators.push_back(operatorsMap["x2"]);
        }

        Context context;

        context.valueMin = minValue;
        context.valueMax = maxValue;
        context.rng.seed(std::random_device{}());

        context.y = std::stod(parts[parts.size() - 1]);
        context.x1 = std::stod(parts[0]);
        if (inputSize > 1) context.x2 = std::stod(parts[1]);

        contexts.push_back(context);
    }

    Penalty penalty;
    GP<double, Context> gp = GP<double, Context>(operators, populationSize, maxDepth, maxNodes, tournamentSize, elitism, pClone, pMutate, pCross, &penalty, contexts);
    gp.initializePopulation();
    
    while (gp.getCostEvaluations() < costEvaluations) {
        gp.newGeneration();
    }

    auto& best = gp.getBestSolution();
    std::cout << "Best penalty: " << best.getPenalty() << '\n';
    std::cout << best.toString() << '\n';

    return 0;
}