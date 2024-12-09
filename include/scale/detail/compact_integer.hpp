/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>

#include <boost/multiprecision/cpp_int.hpp>

#include <scale/detail/fixed_width_integer.hpp>
#include <scale/outcome/outcome_throw.hpp>
#include <scale/scale_error.hpp>
#include <scale/types.hpp>

namespace scale::detail {

  /// min integer encoded by 2 bytes
  constexpr static size_t kMinUint16 = (1ul << 6u);
  /// min integer encoded by 4 bytes
  constexpr static size_t kMinUint32 = (1ul << 14u);
  /// min integer encoded as multibyte
  constexpr static size_t kMinBigInteger = (1ul << 30u);

  /// Returns the compact encoded length for the given value.
  size_t compactLen(std::unsigned_integral auto val) {
    if (val < kMinUint16) return 1;
    if (val < kMinUint32) return 2;
    if (val < kMinBigInteger) return 4;
    size_t counter = 1;
    while ((val >>= 8)) ++counter;
    return counter;
  }

  /**
   * Encodes any integer type to compact-integer representation
   * @tparam T integer type
   * @tparam S output stream type
   * @param value integer value
   * @return byte array representation of value as compact-integer
   */
  template <typename T, typename S>
    requires(std::integral<T> or std::is_same_v<T, CompactInteger>)
  void encodeCompactInteger(T integer, S &s) {
    boost::multiprecision::cpp_int value{integer};

    // cannot encode negative numbers
    // there is no description how to encode compact negative numbers
    if (value < 0) {
      raise(EncodeError::NEGATIVE_COMPACT_INTEGER);
    }

    if (value < kMinUint16) {
      uint8_t v = (value.convert_to<uint8_t>() << 2u) | 0b00;
      return encodeInteger(v, s);
    }

    else if (value < kMinUint32) {
      // only values from [kMinUint16, kMinUint32) can be put here
      uint16_t v = (value.convert_to<uint16_t>() << 2u) | 0b01;
      return encodeInteger(v, s);
    }

    else if (value < kMinBigInteger) {
      // only values from [kMinUint32, kMinBigInteger) can be put here
      uint32_t v = (value.convert_to<uint32_t>() << 2u) | 0b10;
      return encodeInteger(v, s);
    }

    // number of bytes required to represent value
    size_t significant_bytes_n = msb(value) / 8 + 1;

    if (significant_bytes_n > 67) {
      raise(EncodeError::COMPACT_INTEGER_TOO_BIG);
    }

    // The upper 6 bits of the header are used to encode the number of bytes
    // required to store the big integer. The value stored in these 6 bits
    // ranges from 0 to 63 (2^6 - 1). However, the actual byte count starts
    // from 4, so we subtract 4 from the byte count before encoding it.
    // This makes the range of byte counts for storing big integers 4 to 67.
    // To construct the final header, the upper 6 bits are shifted left by
    // 2 positions (equivalent to multiplying by 4).
    // The lower 2 bits (minor bits) store the encoding option, which in this
    // case is 0b11 (decimal value 3). The final header is formed by adding 3
    // to the result of the previous operations.
    uint8_t header = ((significant_bytes_n - 4) << 2u) | 0b11;

    s << header;

    for (auto v = value; v != 0; v >>= 8) {
      // push back the least significant byte
      s << static_cast<uint8_t>(v & 0xff);
    }
  }

  template <typename T, typename S>
    requires std::is_same_v<T, CompactInteger>
  T decodeCompactInteger(S &stream) {
    auto first_byte = stream.nextByte();

    const uint8_t flag = (first_byte)&0b00000011u;

    size_t number = 0u;

    switch (flag) {
      case 0b00u: {
        number = static_cast<size_t>(first_byte >> 2u);
        break;
      }

      case 0b01u: {
        auto second_byte = stream.nextByte();

        number = (static_cast<size_t>((first_byte)&0b11111100u)
                  + static_cast<size_t>(second_byte) * 256u)
                 >> 2u;
        if ((number >> 6) == 0) {
          raise(DecodeError::REDUNDANT_COMPACT_ENCODING);
        }
        break;
      }

      case 0b10u: {
        number = first_byte;
        size_t multiplier = 256u;
        if (!stream.hasMore(3u)) {
          raise(DecodeError::NOT_ENOUGH_DATA);
        }

        for (auto i = 0u; i < 3u; ++i) {
          // we assured that there are 3 more bytes,
          // no need to make checks in a loop
          number += (stream.nextByte()) * multiplier;
          multiplier = multiplier << 8u;
        }
        number = number >> 2u;
        if ((number >> 14) == 0) {
          raise(DecodeError::REDUNDANT_COMPACT_ENCODING);
        }
        break;
      }

      case 0b11: {
        auto bytes_count = ((first_byte) >> 2u) + 4u;
        if (!stream.hasMore(bytes_count)) {
          raise(DecodeError::NOT_ENOUGH_DATA);
        }

        CompactInteger multiplier{1u};
        CompactInteger value = 0;
        // we assured that there are m more bytes,
        // no need to make checks in a loop
        for (auto i = 0u; i < bytes_count; ++i) {
          value += (stream.nextByte()) * multiplier;
          multiplier *= 256u;
        }
        if (value.is_zero()) {
          raise(DecodeError::REDUNDANT_COMPACT_ENCODING);
        }
        auto bits = msb(value) + 1;
        if (bits <= 30 or (bits + 7) / 8 < bytes_count) {
          raise(DecodeError::REDUNDANT_COMPACT_ENCODING);
        }
        return value;  // special case
      }

      default:
        UNREACHABLE
    }

    return CompactInteger{number};
  }

  /**
   * Decodes any integer type from compact-integer representation
   * @tparam T integer type
   * @tparam S input stream type
   * @param value integer value
   * @return value according compact-integer representation
   */
  template <typename T, typename S>
    requires std::unsigned_integral<T>
  T decodeCompactInteger(S &s) {
    auto integer = decodeCompactInteger<CompactInteger>(s);
    if (not integer.is_zero()
        and msb(integer) >= std::numeric_limits<T>::digits) {
      raise(DecodeError::DECODED_VALUE_OVERFLOWS_TARGET);
    }
    return static_cast<T>(integer);
  }


}  // namespace scale::detail
