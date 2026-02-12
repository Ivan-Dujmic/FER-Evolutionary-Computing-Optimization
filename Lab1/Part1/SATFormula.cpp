#include "SATFormula.h"
#include <sstream>

SATFormula::SATFormula(int numberOfVariables, std::vector<Clause> clauses) : numberOfVariables(numberOfVariables), formula(clauses) {}

int SATFormula::getNumberOfVariables() const {
    return numberOfVariables;
}

std::size_t SATFormula::getNumberOfClauses() const {
    return formula.size();
}

Clause SATFormula::getClause(int index) const {
    return formula[index];
}

bool SATFormula::isSatisfied(const BitVector& assignment) const {
    for (Clause clause : formula) {
        if (!clause.isSatisfied(assignment)) return false;
    }

    return true;
}

int SATFormula::nSatisfied(const BitVector& assignment) const {
    int count = 0;
    for (Clause clause : formula) {
        if (clause.isSatisfied(assignment)) count++;
    }

    return count;
}

int SATFormula::whichSatisfied(const BitVector& assignment, std::vector<bool>& which) const {
    int count = 0;

    for (size_t i = 0 ; i < formula.size() ; i++) {
        if (formula[i].isSatisfied(assignment)) {
            which[i] = true; count++;
        }
        else which[i] = false;
    }

    return count;
}

std::string SATFormula::toString() const {
    std::ostringstream oss;
    for (std::size_t i = 0 ; i < formula.size() ; i++) {
        if (i > 0) oss << ' ';
        oss << '(' << formula[i].toString() << ')';
    }

    return oss.str();
}