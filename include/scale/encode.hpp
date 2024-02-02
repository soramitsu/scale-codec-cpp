/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <scale/fn.hpp>
#include <scale/outcome/catch.hpp>
#include <vector>

namespace scale {
  using Bytes = std::vector<uint8_t>;

  outcome::result<void> encodeTo(Bytes &out, const auto &...a) {
    return encodeFn(
        [&](BytesIn raw) {
          // TODO(turuslan): qtils::append(out, raw);
          out.insert(out.end(), raw.begin(), raw.end());
        },
        a...);
  }

  outcome::result<Bytes> encode(const auto &...a) {
    Bytes out;
    OUTCOME_TRY(encodeTo(out, a...));
    return out;
  }

  outcome::result<size_t> encodeSizeOutcome(const auto &...a) {
    size_t n = 0;
    OUTCOME_TRY(encodeFn([&](BytesIn raw) { n += raw.size(); }, a...));
    return n;
  }

  size_t encodeSize(const auto &...a) {
    return encodeSizeOutcome(a...).value();
  }
}  // namespace scale
