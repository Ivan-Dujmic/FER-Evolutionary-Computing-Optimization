#pragma once
#include "Clause.h"

class SATFormula {
    private:
        int numberOfVariables;
        std::vector<Clause> formula;

    public:
        SATFormula(int numberOfVariables, std::vector<Clause> clauses);

        int getNumberOfVariables() const;
        std::size_t getNumberOfClauses() const;
        Clause getClause(int index) const;
        bool isSatisfied(const BitVector& assignment) const;
        int nSatisfied(const BitVector& assignment) const;
        int whichSatisfied(const BitVector& assignment, std::vector<bool>& which) const;
        std::string toString() const;
};