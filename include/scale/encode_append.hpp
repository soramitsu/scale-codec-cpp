/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SCALE_CORE_SCALE_ENCODE_APPEND_HPP
#define SCALE_CORE_SCALE_ENCODE_APPEND_HPP

#include <scale/scale.hpp>

namespace scale {

  /**
   * Vector wrapper, that is scale encoded without prepended CompactInteger
   */
  struct EncodeOpaqueValue {
    gsl::span<const uint8_t> v;
  };

  template <class Stream,
            typename = std::enable_if_t<Stream::is_encoder_stream>>
  Stream &operator<<(Stream &s, const EncodeOpaqueValue &value) {
    for (auto &&it = value.v.begin(), end = value.v.end(); it != end; ++it) {
      s << *it;
    }
    return s;
  }

  /**
   * Adds an EncodeOpaqueValue to a scale encoded vector of EncodeOpaqueValue's.
   * If the current vector is empty, then it is replaced by a new
   * EncodeOpaqueValue
   * In other words, what actually happens could be implemented like that:
   * @code{.cpp}
   * auto vec = scale::decode<vector<EncodeOpaqueValue>>(self_encoded);
   * vec.push_back(scale::encode(EncodeOpaqueValue(input));
   * self_encoded = scale::encode(vec);
   * @endcode
   * but the actual implementation is a bit more optimal
   * @param self_encoded - An encoded vector of EncodeOpaqueValue
   * @param input - A vector encoded as an EncodeOpaqueValue and added to
   * \param self_encoded
   * @return success if input was appended to self_encoded, failure otherwise
   */
  outcome::result<void> append_or_new_vec(std::vector<uint8_t> &self_encoded,
                                          gsl::span<const uint8_t> input);
}  // namespace scale

#endif  // SCALE_CORE_SCALE_ENCODE_APPEND_HPP
