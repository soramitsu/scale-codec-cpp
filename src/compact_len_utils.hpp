/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "scale/types.hpp"

namespace scale::compact {
  // calculate number of bytes required
  inline size_t countBytes(CompactInteger v) {
    size_t counter = 0;
    do {
      ++counter;
    } while ((v >>= 8) != 0);
    return counter;
  }

  /**
   * Returns the compact encoded length for the given value.
   */
  template <typename T,
            typename I = std::decay_t<T>,
            typename = std::enable_if_t<std::is_integral<I>::value>>
  uint32_t compactLen(T val) {
    if (val < EncodingCategoryLimits::kMinUint16) return 1;
    if (val < EncodingCategoryLimits::kMinUint32) return 2;
    if (val < EncodingCategoryLimits::kMinBigInteger) return 4;
    return countBytes(val);
  }
}  // namespace scale::compact
