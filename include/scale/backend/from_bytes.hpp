/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <scale/decoder_backend.hpp>

namespace scale::backend {

  class FromBytes final : public DecoderBackend {
   public:
    FromBytes(std::span<const uint8_t> data) : bytes_(data) {};

    FromBytes(FromBytes &&) noexcept = delete;
    FromBytes(const FromBytes &) = delete;
    FromBytes &operator=(FromBytes &&) noexcept = delete;
    FromBytes &operator=(FromBytes const &) = delete;

    bool has(size_t amount) const override;
    uint8_t take() override;
    void read(std::span<uint8_t> out) override;

   private:
    std::span<const uint8_t> bytes_;
  };

  inline bool FromBytes::has(size_t amount) const {
    return bytes_.size() >= amount;
  }

  inline uint8_t FromBytes::take() {
    auto &&byte = bytes_.front();
    bytes_ = bytes_.last(bytes_.size() - 1);
    return byte;
  }

  inline void FromBytes::read(std::span<uint8_t> out) {
    std::memcpy(out.data(), bytes_.data(), out.size());
    bytes_ = bytes_.last(bytes_.size() - out.size());
  }

}  // namespace scale