/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <vector>

#include <boost/system/system_error.hpp>
#include <boost/throw_exception.hpp>

#include <scale/enum_traits.hpp>
#include <scale/outcome/outcome.hpp>
#include <scale/scale_decoder_stream.hpp>
#include <scale/scale_encoder_stream.hpp>

#define SCALE_EMPTY_DECODER(TargetType)                             \
  template <typename Stream,                                        \
            typename = std::enable_if_t<Stream::is_decoder_stream>> \
  Stream &operator>>(Stream &s, TargetType &) {                     \
    return s;                                                       \
  }

#define SCALE_EMPTY_ENCODER(TargetType)                             \
  template <typename Stream,                                        \
            typename = std::enable_if_t<Stream::is_encoder_stream>> \
  Stream &operator<<(Stream &s, const TargetType &) {               \
    return s;                                                       \
  }

#define SCALE_EMPTY_CODER(TargetType) \
  SCALE_EMPTY_ENCODER(TargetType)     \
  SCALE_EMPTY_DECODER(TargetType)

namespace scale {
  /**
   * @brief convenience function for encoding primitives data to stream
   * @tparam Args primitive types to be encoded
   * @param args data to encode
   * @return encoded data
   */
  template <typename... Args>
  outcome::result<std::vector<uint8_t>> encode(Args &&...args) {
    ScaleEncoderStream s{};
    try {
      (s << ... << std::forward<Args>(args));
    } catch (std::system_error &e) {
      return outcome::failure(e.code());
    }
    return s.to_vector();
  }

  /**
   * @brief convenience function for decoding primitives data from stream
   * @tparam T primitive type that is decoded from provided span
   * @param span of bytes with encoded data
   * @return decoded T
   */
  template <class T>
  outcome::result<T> decode(ConstSpanOfBytes data) {
    T t{};
    ScaleDecoderStream s(data);
    try {
      s >> t;
    } catch (std::system_error &e) {
      return outcome::failure(e.code());
    }

    return outcome::success(std::move(t));
  }
}  // namespace scale
