/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <boost/endian/arithmetic.hpp>

namespace scale {
  class ScaleEncoderStream;
}

namespace scale::detail {

  /**
   * encodeInteger encodes any integer type to little-endian representation
   * @tparam T integer type
   * @param value integer value
   * @param stream output stream
   */
  template <typename T>
    requires std::is_integral_v<std::remove_cvref_t<T>>
  void encodeInteger(T value, ScaleEncoderStream &stream) {
    using I = std::remove_cvref_t<T>;
    constexpr size_t size = sizeof(I);
    constexpr size_t bits = size * 8;
    boost::endian::endian_buffer<boost::endian::order::little, I, bits> buf{};
    buf = value;  // cannot initialize, only assign
    for (size_t i = 0; i < size; ++i) {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      stream << buf.data()[i];
    }
  }

}  // namespace scale::detail
