#pragma once
#include "MutableBitVector.h"
#include <ranges>

class BitVectorNGenerator {
    private:
        BitVector assignment;

    public:
        BitVectorNGenerator(const BitVector& assignment);

        struct iterator {
            const BitVector* assignment;
            std::size_t index;

            iterator& operator++() {
                ++index; return *this;
            }

            bool operator!=(const iterator& other) const {
                return index != other.index;
            }

            MutableBitVector operator*() const {
                MutableBitVector copy = assignment->copy();
                copy.set(index, !assignment->get(index));
                return copy;
            }
        };

        iterator begin() const;
        iterator end() const;

        std::vector<MutableBitVector> createNeighbourhood();
};