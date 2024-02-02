/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <scale/enum_traits.hpp>
#include <scale/outcome/catch.hpp>
#include <scale/scale_decoder_stream.hpp>

namespace scale {
  /**
   * @brief convenience function for decoding primitives data from stream
   * @tparam T primitive type that is decoded from provided span
   * @param span of bytes with encoded data
   * @return decoded T
   */
  template <class T>
  outcome::result<T> decode(ConstSpanOfBytes data) {
    ScaleDecoderStream s(data);
    return decode<T>(s);
  }
  template <typename T>
  outcome::result<T> decode(ScaleDecoderStream &s) {
    T t{};
    OUTCOME_TRY(decode<T>(s, t));
    return outcome::success(std::move(t));
  }
  template <typename T>
  outcome::result<void> decode(ScaleDecoderStream &s, T &t) {
    return outcomeCatch([&] { s >> t; });
  }
}  // namespace scale
