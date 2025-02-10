/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>
#include <span>

namespace scale {

  class DecoderBackend {
   public:
    virtual ~DecoderBackend() = default;

    [[nodiscard]] virtual bool has(size_t amount) const = 0;
    virtual uint8_t take() = 0;
    virtual void read(std::span<uint8_t> out) = 0;
  };

}  // namespace scale