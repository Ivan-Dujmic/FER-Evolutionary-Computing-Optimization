#pragma once
#include <vector>
#include "BitVector.h"

class Clause {
    private:
        std::vector<int> clause;
        
    public:
        Clause(const std::vector<int>& indices);

        std::size_t getSize() const;
        int getLiteral(int index) const;
        bool isSatisfied(const BitVector& assignment) const;
        std::string toString() const;
};