/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>

#include <boost/endian/arithmetic.hpp>

#include <scale/outcome/outcome_throw.hpp>
#include <scale/scale_error.hpp>
#include <scale/unreachable.hpp>

namespace scale::detail {
  /**
   * encodeInteger encodes any integer type to little-endian representation
   * @tparam T integer type
   * @tparam S output stream type
   * @param value integer value
   * @return byte array representation of value
   */
  template <class T,
            class S,
            typename I = std::decay_t<T>,
            typename = std::enable_if_t<std::is_integral<I>::value>>
  void encodeInteger(T value, S &out) {  // no need to take integers by &&
    constexpr size_t size = sizeof(I);
    constexpr size_t bits = size * 8;
    boost::endian::endian_buffer<boost::endian::order::little, I, bits> buf{};
    buf = value;  // cannot initialize, only assign
    for (size_t i = 0; i < size; ++i) {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      out << buf.data()[i];
    }
  }
}  // namespace scale::detail
