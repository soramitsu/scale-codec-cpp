/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SCALE_SCALE_TYPES_HPP
#define SCALE_SCALE_TYPES_HPP

#include <vector>

#include <boost/multiprecision/cpp_int.hpp>
#include <scale/outcome/outcome.hpp>

namespace scale {
  /**
   * @brief convenience alias for arrays of bytes
   */
  using ByteArray = std::vector<uint8_t>;
  /**
   * @brief represents compact integer value
   */
  using CompactInteger = boost::multiprecision::cpp_int;

  /**
   * @brief OptionalBool is internal extended bool type
   */
  enum class OptionalBool : uint8_t { NONE = 0u, OPT_TRUE = 1u, OPT_FALSE = 2u };
}  // namespace scale

namespace scale::compact {
  /**
   * @brief categories of compact encoding
   */
  struct EncodingCategoryLimits {
    // min integer encoded by 2 bytes
    constexpr static size_t kMinUint16 = (1ul << 6u);
    // min integer encoded by 4 bytes
    constexpr static size_t kMinUint32 = (1ul << 14u);
    // min integer encoded as multibyte
    constexpr static size_t kMinBigInteger = (1ul << 30u);
  };
}  // namespace scale::compact
#endif  // SCALE_SCALE_TYPES_HPP
