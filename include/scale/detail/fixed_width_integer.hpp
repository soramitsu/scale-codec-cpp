/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SCALE_SCALE_UTIL_HPP
#define SCALE_SCALE_UTIL_HPP

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

  /**
   * @brief decodeInteger function decodes integer from stream
   * @tparam T integer type
   * @param stream source stream
   * @return decoded value or error
   */
  template <class T,
            class S,
            typename I = std::decay_t<T>,
            typename = std::enable_if_t<std::is_integral_v<I>>>
  I decodeInteger(S &stream) {
    constexpr size_t size = sizeof(I);
    static_assert(size <= 8);

    // sign bit = 2^(num_bits - 1)
    static constexpr auto sign_bit = [](size_t num_bytes) -> uint64_t {
      return 0x80ul << (num_bytes * 8);
    };

    static constexpr auto multiplier = [](size_t num_bytes) -> uint64_t {
      return 0x1ul << (num_bytes * 8);
    };

    if (!stream.hasMore(size)) {
      raise(DecodeError::NOT_ENOUGH_DATA);
      UNREACHABLE
    }

    // get integer as 8 bytes from little-endian stream
    // and represent it as native-endian unsigned integer
    uint64_t v = 0u;

    for (size_t i = 0; i < size && stream.hasMore(1); ++i) {
      v += multiplier(i) * static_cast<uint64_t>(stream.nextByte());
    }
    // now we have an uint64 native-endian value
    // which can store either a signed or an unsigned value

    // if the value is actually unsigned, we know that is not greater than
    // the max value for type T, so static_cast<T>(v) is safe

    // if it is signed and positive, it is also ok
    // we can be sure that it is less than max_value<T>/2.
    // To check if it is negative we check if the sign bit is present
    // in the unsigned representation. (which is true when the value is greater
    // than  2^(size_in_bits-1)
    bool is_positive_signed = v < sign_bit(size - 1);
    if (std::is_unsigned<I>() || is_positive_signed) {
      return static_cast<I>(v);
    }

    // T is a signed integer type and the value v is negative.
    // A value is negative, which means that (-x),
    // where (-x) is positive, is smaller than sign_bits[size-1].
    // Find this x, safely cast to a positive signed and negate the result.
    // the bitwise negation operation affects higher bits as well,
    // but it doesn't spoil the result.
    // static_cast to a smaller type cuts these higher bits off.
    I sv = -static_cast<I>((~v) + 1);
    return sv;
  }
} // namespace scale::detail

#endif // SCALE_SCALE_UTIL_HPP
