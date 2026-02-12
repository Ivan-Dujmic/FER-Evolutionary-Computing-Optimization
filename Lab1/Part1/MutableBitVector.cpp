#include "MutableBitVector.h"

MutableBitVector::MutableBitVector(const boost::dynamic_bitset<>& bits) : BitVector(bits) {}

MutableBitVector::MutableBitVector(int n) : BitVector(n) {}

void MutableBitVector::set(int index, bool value) {
    bits[bits.size() - 1 - index] = value;
}

bool MutableBitVector::increment() {
    bool carry = true;
    for (std::size_t i = 0 ; i < bits.size() && carry ; i++) {
        carry = bits[i];
        bits[i] = !carry;
    }
    return carry;
}