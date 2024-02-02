/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cstring>
#include <scale/encode.hpp>
#include <scale/encode_append.hpp>
#include <scale/scale.hpp>

namespace scale {
  outcome::result<void> append_or_new_vec(Bytes &encoded,
                                          BytesIn items_raw,
                                          size_t items_count) {
    // No data present, just encode the given input data.
    if (encoded.empty()) {
      EncodeCompact count{items_count};
      encoded.reserve(encodeSize(count) + items_raw.size());
      OUTCOME_TRY(encodeTo(encoded, count, EncodeRaw{items_raw}));
      return outcome::success();
    }
    OUTCOME_TRY(old_count_big, decode<CompactInteger>(encoded));
    auto old_count = size_t{old_count_big};
    size_t new_count = old_count + items_count;
    auto old_count_len = encodeSize(EncodeCompact{old_count});
    auto new_count_len = encodeSize(EncodeCompact{new_count});
    auto old = encoded.size() - old_count_len;
    encoded.reserve(new_count_len + old + items_raw.size());
    if (new_count_len != old_count_len) {
      encoded.resize(new_count_len + old);
      std::memmove(
          encoded.data() + new_count_len, encoded.data() + old_count_len, old);
    }
    auto it = encoded.begin();
    encodeCompact(
        FnTag{},
        [&](BytesIn raw) { it = std::copy(raw.begin(), raw.end(), it); },
        new_count);
    OUTCOME_TRY(encodeTo(encoded, EncodeRaw{items_raw}));
    return outcome::success();
  }
}  // namespace scale
