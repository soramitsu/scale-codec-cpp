/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <scale/encoder_backend.hpp>

#include <deque>

namespace scale::backend {

  class ToBytes final : public EncoderBackend {
   public:
    ToBytes() = default;
    ToBytes(ToBytes &&) noexcept = delete;
    ToBytes(const ToBytes &) = delete;
    ToBytes &operator=(ToBytes &&) noexcept = delete;
    ToBytes &operator=(ToBytes const &) = delete;

    void put(uint8_t byte) override;

    void write(std::span<const uint8_t> byte) override;

    [[nodiscard]] size_t size() const override {
      return bytes_.size();
    }

    [[nodiscard]] std::vector<uint8_t> to_vector() const & {
      return {bytes_.begin(), bytes_.end()};
    }

    [[nodiscard]] std::vector<uint8_t> to_vector() && {
      return {std::make_move_iterator(bytes_.begin()),
              std::make_move_iterator(bytes_.end())};
    }

   private:
    std::deque<uint8_t> bytes_;
  };

  inline void ToBytes::put(uint8_t byte) {
    bytes_.push_back(byte);
  }

  inline void ToBytes::write(std::span<const uint8_t> bytes) {
    bytes_.insert(bytes_.end(), bytes.begin(), bytes.end());
  }
}  // namespace scale