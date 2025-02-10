/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <scale/encoder_backend.hpp>

namespace scale::backend {

  class ForCount final : public EncoderBackend {
   public:
    ForCount() = default;
    ForCount(ForCount &&) noexcept = delete;
    ForCount(const ForCount &) = delete;
    ForCount &operator=(ForCount &&) noexcept = delete;
    ForCount &operator=(ForCount const &) = delete;

    void put(uint8_t byte) override;

    void write(std::span<const uint8_t> bytes) override;

    [[nodiscard]] size_t size() const override;

   private:
    size_t count_{0};
  };

  inline void ForCount::put(uint8_t byte) {
    ++count_;
  }

  inline void ForCount::write(std::span<const uint8_t> bytes) {
    count_ += bytes.size();
  }

  inline size_t ForCount::size() const {
    return count_;
  }
}  // namespace scale