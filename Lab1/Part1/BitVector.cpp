#include "BitVector.h"
#include "MutableBitVector.h"

BitVector::BitVector(std::mt19937 rand, int numberOfBits) {
    static std::mt19937 rng(rand);
    static std::uniform_int_distribution<> dist(0, 1);

    bits.resize(numberOfBits);

    for (std::size_t i = 0 ; i < bits.size() ; i++) {
        bits[i] = dist(rng);
    }
}

BitVector::BitVector(const boost::dynamic_bitset<>& bits) : bits(bits) {}

BitVector::BitVector(int n) {
    bits.resize(n);
}

bool BitVector::get(int index) const {
    return bits[bits.size() - 1 - index];
}

std::size_t BitVector::getSize() const {
    return bits.size();
}

std::string BitVector::toString() const {
    std::string buffer;
    boost::to_string(bits, buffer);
    return buffer;
}

MutableBitVector BitVector::copy() const {
    return MutableBitVector(bits);
}