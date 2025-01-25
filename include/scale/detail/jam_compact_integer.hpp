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

#include <scale/outcome/outcome_throw.hpp>
#include <scale/scale_error.hpp>
#include <scale/types.hpp>

namespace scale::detail {

  /**
   * Encodes any integer type to jam-compact-integer representation
   * @tparam T integer type
   * @tparam S output stream type
   * @param value integer value
   * @return byte array representation of value as jam-compact-integer
   */
  template <typename T, typename S>
    requires(std::unsigned_integral<std::decay_t<T>>
             or std::is_same_v<std::decay_t<T>, CompactInteger>)
  void encodeJamCompactInteger(T integer, S &s) {
    size_t value;

    if constexpr (std::is_same_v<T, CompactInteger>) {
      // cannot encode negative numbers
      // there is no description how to encode compact negative numbers
      if (integer < 0) {
        raise(EncodeError::NEGATIVE_COMPACT_INTEGER);
      }
      if (integer.is_zero()) {
        value = 0;
      } else {
        if (msb(integer) >= std::numeric_limits<size_t>::digits) {
          raise(EncodeError::VALUE_TOO_BIG_FOR_COMPACT_REPRESENTATION);
        }
        value = integer.template convert_to<size_t>();
      }
    } else {
      value = static_cast<size_t>(integer);
    }

    if (value < 0x80) {
      s << static_cast<uint8_t>(value);
      return;
    }

    std::array<uint8_t, sizeof(size_t) + 1> bytes;
    uint8_t &prefix = bytes[0] = 0;
    size_t len = 1;

    for (decltype(value) i = value; i != 0; i >>= 8) {
      // minimal value in prefix
      if (i <= (static_cast<uint8_t>(~prefix) >> 1)) {
        prefix |= i;
        break;
      }
      prefix = (prefix >> 1) | 0x80;
      bytes[len++] = static_cast<uint8_t>(i & 0xff);
    }

    for (auto byte : bytes) {
      s << byte;
      if (--len == 0) break;
    }
  }

  /**
   * Decodes any integer type from jam-compact-integer representation
   * @tparam T integer type
   * @tparam S input stream type
   * @param value integer value
   * @return value according jam-compact-integer representation
   */
  template <typename T>
    requires std::unsigned_integral<T> or std::is_same_v<T, CompactInteger>
  T decodeJamCompactInteger(auto &s) {
    uint8_t byte;

    s >> byte;

    if (byte == 0) {
      return 0;
    }

    uint8_t len_bits = byte;
    uint8_t val_bits = byte;
    uint8_t val_mask = 0xff;

    size_t value = 0;

    for (uint8_t i = 0; i < 8; ++i) {
      val_mask >>= 1;
      val_bits &= val_mask;

      if ((len_bits & static_cast<uint8_t>(0x80))
          == 0) {  // no more significant bytes
        value |= static_cast<size_t>(val_bits) << (8 * i);
        break;
      }
      len_bits <<= 1;

      s >> byte;
      value |= static_cast<size_t>(byte) << (8 * i);
    }
    if (val_bits == 0 and (byte & ~val_mask) == 0) {
      raise(DecodeError::REDUNDANT_COMPACT_ENCODING);
    }

    if constexpr (not std::is_same_v<T, size_t>
                  and not std::is_same_v<T, CompactInteger>) {
      if (value > std::numeric_limits<T>::max()) {
        raise(DecodeError::DECODED_VALUE_OVERFLOWS_TARGET);
      }
    }

    return static_cast<T>(value);
  }

}  // namespace scale::detail
