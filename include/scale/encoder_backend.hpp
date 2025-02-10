/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */
 
#pragma once

#include <cstdint>
#include <span>

namespace scale { 

class EncoderBackend
{
  public:
    virtual ~EncoderBackend() = default;

    virtual void put(uint8_t byte) = 0;
    virtual void write(std::span<const uint8_t> byte) = 0;
    [[nodiscard]] virtual size_t size() const = 0;
};

} // scale