#pragma once
#include "BitVector.h"
#include <optional>

class IOptAlgorithm {
    public:
        virtual ~IOptAlgorithm() = default;

        virtual std::optional<BitVector> solve(const std::optional<BitVector>& initial) = 0;
};