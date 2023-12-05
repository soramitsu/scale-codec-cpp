/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <vector>

namespace scale {
  /**
   * @brief Bit vector encoding compatible with rust `BitVec<u8, Lsb0>`
   */
  struct BitVec {
    std::vector<bool> bits;

    bool operator==(const BitVec &other) const {
      return bits == other.bits;
    }
  };
}  // namespace scale
