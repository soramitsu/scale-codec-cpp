/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SCALE_BITVEC_HPP
#define SCALE_BITVEC_HPP

#include <vector>

namespace scale {
  /**
   * @brief convenience alias for arrays of bytes
   */
  struct BitVec {
    std::vector<bool> bits;

    bool operator==(const BitVec &other) const {
      return bits == other.bits;
    }
    bool operator!=(const BitVec &other) const {
      return !(*this == other);
    }
  };
}  // namespace scale

#endif  // SCALE_BITVEC_HPP
