#pragma once
#include <random>
#include <string>
#include <boost/dynamic_bitset.hpp>

class MutableBitVector;

class BitVector {
    protected:
        boost::dynamic_bitset<> bits;

    public:
        BitVector(std::mt19937 rand, int numberOfBits);
        BitVector(const boost::dynamic_bitset<>& bits);
        BitVector(int n);
        
        bool get(int index) const;
        std::size_t getSize() const;
        std::string toString() const;
        MutableBitVector copy() const;
};