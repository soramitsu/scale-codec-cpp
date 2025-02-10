/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <deque>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include <qtils/tagged.hpp>

#include <scale/bitvec.hpp>
// ReSharper disable once CppUnusedIncludeDirective
#include <scale/detail/decompose_and_apply.hpp>
#include <scale/detail/fixed_width_integer.hpp>
#include <scale/detail/compact_integer.hpp>

#include <qtils/outcome.hpp>

#include <scale/configurable.hpp>
#include <scale/encoder_backend.hpp>
#include <scale/scale_error.hpp>

namespace scale {

  template <typename EncoderBackendT>
    requires std::derived_from<EncoderBackendT, EncoderBackend>
  class Encoder : public Configurable {
   public:
    using BackendType = EncoderBackendT;

    auto &backend() {
      return backend_;
    }

    Encoder() = default;

#ifdef CUSTOM_CONFIG_ENABLED
    explicit Encoder(const MaybeConfig auto &...configs)
        : Configurable(configs...) {}
#else
    [[deprecated("Scale has compiled without custom config support")]]  //
    explicit Encoder(const MaybeConfig auto &...configs) = delete;
#endif

    template <typename T>
    Encoder &operator<<(T &&value) {
      encode(std::forward<T>(value), *this);
      return *this;
    }

    void put(uint8_t byte) {
      backend_.put(byte);
    }

    void write(std::span<const uint8_t> byte) {
      backend_.write(byte);
    }

    [[nodiscard]] size_t size() const {
      return backend_.size();
    }

   private:
    EncoderBackendT backend_;
  };

  using detail::decompose_and_apply;

  void encode(Decomposable auto &&decomposable, ScaleEncoder auto &encoder)
    requires NoTagged<decltype(decomposable)>
  {
    decompose_and_apply(std::forward<decltype(decomposable)>(decomposable),
                        [&](auto &...args) { (encode(args, encoder), ...); });
  }

  void encode(const std::string_view view, ScaleEncoder auto &encoder)
    requires NoTagged<decltype(view)>
  {
    encode(as_compact(view.size()), encoder);
    for (auto &item : view) {
      encoder << static_cast<uint8_t>(item);
    }
  }

  void encode(const std::nullopt_t &, ScaleEncoder auto &encoder) {
    encoder.put(0);
  }

  void encode(DynamicCollection auto &&collection, ScaleEncoder auto &encoder)
    requires NoTagged<decltype(collection)>
             and (not std::same_as<std::remove_cvref_t<decltype(collection)>,
                                   std::vector<bool>>)
             and (not std::same_as<std::remove_cvref_t<decltype(collection)>,
                                   struct Object>)
             and (not Decomposable<std::remove_cvref_t<decltype(collection)>>)
  {
    encode(as_compact(collection.size()), encoder);
    for (auto &&item : std::forward<decltype(collection)>(collection)) {
      encode(item, encoder);
    }
  }

  void encode(StaticCollection auto &&collection, ScaleEncoder auto &encoder)
    requires NoTagged<decltype(collection)>
             and (not DecomposableArray<decltype(collection)>)
  {
    for (auto &&item : std::forward<decltype(collection)>(collection)) {
      encode(item, encoder);
    }
  }

  template <typename T>
    requires std::same_as<std::remove_cvref_t<T>, BitVec>
  void encode(T &&bit_vector, ScaleEncoder auto &encoder)
  // requires NoTagged<decltype(bit_vector)>
  {
    encode(as_compact(bit_vector.bits.size()), encoder);
    size_t i = 0;
    uint8_t byte = 0;
    for (auto bit : bit_vector.bits) {
      if (bit) {
        byte |= 1 << (i % 8);
      }
      ++i;
      if (i % 8 == 0) {
        encoder.put(byte);
        byte = 0;
      }
    }
    if (i % 8 != 0) {
      encoder.put(byte);
    }
  }

  template <typename T>
    requires std::same_as<std::remove_cvref_t<T>, std::vector<bool>>
  void encode(T &&vector, ScaleEncoder auto &encoder)
    requires NoTagged<decltype(vector)>
  {
    encode(as_compact(vector.size()), encoder);
    for (const bool item : vector) {
      encoder.put(static_cast<uint8_t>(item ? 1 : 0));
    }
  }

  void encode(qtils::is_tagged_v auto &&tagged, ScaleEncoder auto &encoder)
    requires(not CompactInteger<decltype(tagged)>)
  {
    encode(untagged(tagged), encoder);
  }

}  // namespace scale
