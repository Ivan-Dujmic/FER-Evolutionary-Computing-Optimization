#pragma once
#include "BitVector.h"

class MutableBitVector : public BitVector {
    public:
        MutableBitVector(const boost::dynamic_bitset<>& bits);
        MutableBitVector(int n);

        void set(int index, bool value);
        bool increment();
};