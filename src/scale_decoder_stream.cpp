/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <scale/scale_decoder_stream.hpp>

namespace scale {
  namespace {
    CompactInteger decodeCompactInteger(ScaleDecoderStream &stream) {
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
  }  // namespace

  size_t ScaleDecoderStream::decodeLength() {
    size_t size = decodeCompact<size_t>();
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
    v = decodeCompactInteger(*this);
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
