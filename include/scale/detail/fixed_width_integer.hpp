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
   * @tparam S output stream type, derived from ScaleEncoderStream
   * @param value integer value
   * @param stream output stream
   */
  template <typename T, typename S>
    requires std::is_integral_v<std::decay_t<T>>
             && std::is_base_of_v<ScaleEncoderStream, S>
  void encodeInteger(T value, S &stream) {
    using I = std::decay_t<T>;
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
