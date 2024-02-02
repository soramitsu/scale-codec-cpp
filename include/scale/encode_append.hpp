/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>
#include <scale/outcome/outcome.hpp>
#include <span>
#include <vector>

namespace scale {
  using BytesIn = std::span<const uint8_t>;
  using Bytes = std::vector<uint8_t>;

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
  outcome::result<void> append_or_new_vec(Bytes &encoded,
                                          BytesIn items_raw,
                                          size_t items_count);
}  // namespace scale
