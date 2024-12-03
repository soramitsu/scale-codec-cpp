/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <scale/scale_decoder_stream.hpp>

#include <scale/detail/fixed_width_integer.hpp>
#include <scale/detail/jam_compact_integer.hpp>

namespace scale {
  size_t ScaleDecoderStream::decodeLength() {
#ifdef JAM_COMPATIBILITY_ENABLED
    size_t size = detail::decodeJamCompactInteger<size_t>(*this);
#else
    size_t size = detail::decodeCompactInteger<size_t>(*this);
#endif
    if (not hasMore(size)) {
      raise(DecodeError::NOT_ENOUGH_DATA);
    }
    return size;
  }

  std::optional<bool> ScaleDecoderStream::decodeOptionalBool() {
    auto byte = nextByte();
    switch (static_cast<OptionalBool>(byte)) {
      case OptionalBool::NONE:
        return std::nullopt;
      case OptionalBool::OPT_FALSE:
        return false;
      case OptionalBool::OPT_TRUE:
        return true;
    }
    raise(DecodeError::UNEXPECTED_VALUE);
  }

  bool ScaleDecoderStream::decodeBool() {
    auto byte = nextByte();
    switch (byte) {
      case 0u:
        return false;
      case 1u:
        return true;
      default:
        raise(DecodeError::UNEXPECTED_VALUE);
    }
  }

  ScaleDecoderStream &ScaleDecoderStream::operator>>(CompactInteger &v) {
    v = decodeCompact<CompactInteger>();
    return *this;
  }

  ScaleDecoderStream &ScaleDecoderStream::operator>>(BitVec &v) {
    auto size = decodeCompact<size_t>();
    if (not hasMore((size + 7) / 8)) {
      raise(DecodeError::NOT_ENOUGH_DATA);
    }
    v.bits.resize(size);
    size_t i = 0;
    uint8_t byte = 0;
    for (std::vector<bool>::reference bit : v.bits) {
      if (i % 8 == 0) {
        byte = nextByte();
      }
      bit = ((byte >> (i % 8)) & 1) != 0;
      ++i;
    }

    return *this;
  }

  bool ScaleDecoderStream::hasMore(uint64_t n) const {
    return static_cast<size_t>(current_index_ + n) <= span_.size();
  }

  uint8_t ScaleDecoderStream::nextByte() {
    if (not hasMore(1)) {
      raise(DecodeError::NOT_ENOUGH_DATA);
    }
    return span_[current_index_++];
  }
}  // namespace scale
