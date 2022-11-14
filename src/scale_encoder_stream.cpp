/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "scale/scale_encoder_stream.hpp"

#include "compact_len_utils.hpp"
#include "scale/scale_error.hpp"
#include "scale/types.hpp"

namespace scale {
  namespace {
    // must not use these functions outside encodeInteger
    void encodeFirstCategory(uint8_t value, ScaleEncoderStream &out) {
      // only values from [0, kMinUint16) can be put here
      out << static_cast<uint8_t>(value << 2u);
    }

    void encodeSecondCategory(uint16_t value, ScaleEncoderStream &out) {
      // only values from [kMinUint16, kMinUint32) can be put here
      auto v = value;
      v <<= 2u;  // v *= 4
      v += 1u;   // set 0b01 flag
      auto minor_byte = static_cast<uint8_t>(v & 0xFFu);
      v >>= 8u;
      auto major_byte = static_cast<uint8_t>(v & 0xFFu);

      out << minor_byte << major_byte;
    }

    void encodeThirdCategory(uint32_t value, ScaleEncoderStream &out) {
      // only values from [kMinUint32, kMinBigInteger) can be put here
      uint32_t v = (value << 2u) + 2;
      scale::detail::encodeInteger<uint32_t>(v, out);
    }

    /**
     * @brief compact-encodes CompactInteger
     * @param value source CompactInteger value
     */
    void encodeCompactInteger(const CompactInteger &value,
                              ScaleEncoderStream &out) {
      // cannot encode negative numbers
      // there is no description how to encode compact negative numbers
      if (value < 0) {
        raise(EncodeError::NEGATIVE_COMPACT_INTEGER);
      }

      if (value < compact::EncodingCategoryLimits::kMinUint16) {
        encodeFirstCategory(value.convert_to<uint8_t>(), out);
        return;
      }

      if (value < compact::EncodingCategoryLimits::kMinUint32) {
        encodeSecondCategory(value.convert_to<uint16_t>(), out);
        return;
      }

      if (value < compact::EncodingCategoryLimits::kMinBigInteger) {
        encodeThirdCategory(value.convert_to<uint32_t>(), out);
        return;
      }

      // number of bytes required to represent value
      size_t bigIntLength = compact::countBytes(value);

      // number of bytes to scale-encode value
      // 1 byte is reserved for header
      size_t requiredLength = 1 + bigIntLength;

      if (bigIntLength > 67) {
        raise(EncodeError::COMPACT_INTEGER_TOO_BIG);
      }

      ByteArray result;
      result.reserve(requiredLength);

      /* The value stored in 6 major bits of header is used
       * to encode number of bytes for storing big integer.
       * Value formed by 6 bits varies from 0 to 63 == 2^6 - 1,
       * However big integer byte count starts from 4,
       * so to store this number we should decrease this value by 4.
       * And the range of bytes number for storing big integer
       * becomes 4 .. 67. To form resulting header we need to move
       * those bits representing bytes count to the left by 2 positions
       * by means of multiplying by 4.
       * Minor 2 bits store encoding option, in our case it is 0b11 == 3
       * We just add 3 to the result of operations above
       */
      uint8_t header = (bigIntLength - 4) * 4 + 3;

      result.push_back(header);

      CompactInteger v{value};
      for (size_t i = 0; i < bigIntLength; ++i) {
        result.push_back(static_cast<uint8_t>(
            v & 0xFF));  // push back least significant byte
        v >>= 8;
      }

      for (const uint8_t c : result) {
        out << c;
      }
    }
  }  // namespace

  ScaleEncoderStream::ScaleEncoderStream()
      : drop_data_{false}, bytes_written_{0} {}

  ScaleEncoderStream::ScaleEncoderStream(bool drop_data)
      : drop_data_{drop_data}, bytes_written_{0} {}

  ByteArray ScaleEncoderStream::to_vector() const {
    ByteArray buffer(stream_.size(), 0u);
    for (auto &&[it, dest] = std::pair(stream_.begin(), buffer.begin());
         it != stream_.end();
         ++it, ++dest) {
      *dest = *it;
    }
    return buffer;
  }

  size_t ScaleEncoderStream::size() const {
    return bytes_written_;
  }

  ScaleEncoderStream &ScaleEncoderStream::operator<<(const BitVec &v) {
    *this << CompactInteger{v.bits.size()};
    size_t i = 0;
    uint8_t byte = 0;
    for (auto bit : v.bits) {
      if (bit) {
        byte |= 1 << (i % 8);
      }
      ++i;
      if (i % 8 == 0) {
        putByte(byte);
        byte = 0;
      }
    }
    if (i % 8 != 0) {
      putByte(byte);
    }
    return *this;
  }

  ScaleEncoderStream &ScaleEncoderStream::putByte(uint8_t v) {
    ++bytes_written_;
    if (!drop_data_) {
      stream_.push_back(v);
    }
    return *this;
  }

  ScaleEncoderStream &ScaleEncoderStream::operator<<(const CompactInteger &v) {
    encodeCompactInteger(v, *this);
    return *this;
  }

  ScaleEncoderStream &ScaleEncoderStream::encodeOptionalBool(
      const std::optional<bool> &v) {
    auto result = OptionalBool::OPT_TRUE;

    if (!v.has_value()) {
      result = OptionalBool::NONE;
    } else if (!*v) {
      result = OptionalBool::OPT_FALSE;
    }

    return putByte(static_cast<uint8_t>(result));
  }

}  // namespace scale
