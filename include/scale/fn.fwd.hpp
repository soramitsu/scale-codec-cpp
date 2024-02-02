/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>
#include <span>

#define _SCALE_FN const ::scale::Fn auto &fn
#define SCALE_FN const ::scale::FnTag &tag, _SCALE_FN

namespace scale {
  using BytesIn = std::span<const uint8_t>;

  struct FnTag {};

  template <typename F>
  concept Fn = requires(F f) { f(BytesIn{}); };

  struct EncodeCompact {
    uint64_t v;

    void tie();
  };

  struct EncodeRaw {
    BytesIn v;

    void tie();
  };
}  // namespace scale
