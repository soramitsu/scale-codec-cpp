/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include <scale/outcome/outcome_throw.hpp>
#include <scale/scale_error.hpp>
#include <scale/types.hpp>

namespace scale::detail {

  /// Returns the compact encoded length for the given value.
  size_t lengthOfEncodedJamCompactInteger(CompactCompatible auto value) {
    if constexpr (std::unsigned_integral<decltype(value)>) {
      return 1 + (std::bit_width(value) - 1) / 7;
    } else {
      return 1 + msb(value - 1) / 7;
    }
  }

  /**
   * Encodes any integer type to jam-compact-integer representation
   * @tparam T integer type
   * @return byte array representation of value as jam-compact-integer
   */
  template <typename T>
    requires CompactCompatible<std::remove_cvref_t<T>>
  void encodeJamCompactInteger(T &&integer, ScaleEncoder auto &encoder) {
    constexpr auto is_integral = std::unsigned_integral<std::remove_cvref_t<T>>;

    size_t value;

    if constexpr (is_integral) {
      value = static_cast<size_t>(integer);
    } else {
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
    }

    if (value < 0x80) {
      encoder.put(static_cast<uint8_t>(value));
      return;
    }

    // NOLINTNEXTLINE(*-pro-type-member-init)
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
      encoder.put(byte);
      if (--len == 0) break;
    }
  }

  /**
   * Decodes any integer type from jam-compact-integer representation
   * @return value according jam-compact-integer representation
   */
  boost::multiprecision::uint128_t decodeJamCompactInteger(
      ScaleDecoder auto &decoder) {
    uint8_t byte;

    byte = decoder.take();

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

      byte = decoder.take();
      value |= static_cast<size_t>(byte) << (8 * i);
    }
    if (val_bits == 0 and (byte & ~val_mask) == 0) {
      raise(DecodeError::REDUNDANT_COMPACT_ENCODING);
    }
    return value;
  }

}  // namespace scale::detail
