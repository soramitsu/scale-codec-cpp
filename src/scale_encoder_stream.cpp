/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <scale/scale_encoder_stream.hpp>

namespace scale {
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
    *this << Length(v.bits.size());
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
    if (not drop_data_) {
      stream_.push_back(v);
    }
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
