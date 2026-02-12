#include "Clause.h"
#include <sstream>

Clause::Clause(const std::vector<int>& indices) : clause(indices) {}

std::size_t Clause::getSize() const {
    return clause.size();
}

int Clause::getLiteral(int index) const {
    return clause[index];
}

bool Clause::isSatisfied(const BitVector& assignment) const {
    for (int literal : clause) {
        if (literal > 0) {
            if (assignment.get(literal-1)) return true;
        } else {
            if (!assignment.get(abs(literal)-1)) return true;
        }
    }

    return false;
}

std::string Clause::toString() const {
    std::ostringstream oss;

    for (std::size_t i = 0 ; i < clause.size() ; i++) {
        if (i > 0) oss << " ";
        oss << clause[i];
    }
    
    return oss.str();
}