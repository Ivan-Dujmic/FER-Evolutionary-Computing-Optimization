#include "BitVectorNGenerator.h"

BitVectorNGenerator::BitVectorNGenerator(const BitVector& assignment) : assignment(assignment) {}

BitVectorNGenerator::iterator BitVectorNGenerator::begin() const { 
    return iterator{&assignment, 0}; 
}
BitVectorNGenerator::iterator BitVectorNGenerator::end() const { 
    return iterator{&assignment, assignment.getSize()}; 
}

std::vector<MutableBitVector> BitVectorNGenerator::createNeighbourhood() {
    std::vector<MutableBitVector> neighbourhood;
    for (std::size_t i = 0 ; i < assignment.getSize() ; i++) {
        MutableBitVector newBitVector = assignment.copy();
        newBitVector.set(i, !assignment.get(i));
        neighbourhood.push_back(newBitVector);
    }
    return neighbourhood;
}