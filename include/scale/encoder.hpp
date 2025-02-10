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
#include <scale/definitions.hpp>
#include <scale/detail/decompose_and_apply.hpp>
#include <scale/detail/fixed_width_integer.hpp>
#ifdef JAM_COMPATIBILITY_ENABLED
#include <scale/detail/jam_compact_integer.hpp>
#else
#include <scale/detail/compact_integer.hpp>
#endif

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


  void encode(const IsEnum auto &enumeration, ScaleEncoder auto &encoder)
    requires NoTagged<decltype(enumeration)>
  {
    using T =
        std::underlying_type_t<std::remove_cvref_t<decltype(enumeration)>>;
    encode(static_cast<T>(enumeration), encoder);
  }

  template <typename T>
  void encode(const std::shared_ptr<T> &sptr, ScaleEncoder auto &encoder)
    requires NoTagged<decltype(sptr)>
  {
    if (sptr) {
      return encode(*sptr, encoder);
    }
    raise(EncodeError::DEREF_NULLPOINTER);
  }

  template <typename T>
  void encode(const std::unique_ptr<T> &uptr, ScaleEncoder auto &encoder)
    requires NoTagged<decltype(uptr)>
  {
    if (uptr) {
      return encode(*uptr, encoder);
    }
    raise(EncodeError::DEREF_NULLPOINTER);
  }

  /**
   * @brief scale-encodes std::reference_wrapper of a type
   * @return reference to stream;
   */
  template <typename T>
  void encode(const std::reference_wrapper<T> &reference,
              ScaleEncoder auto &encoder)
    requires NoTagged<decltype(reference)>
  {
    encode(reference.get(), encoder);
  }

  using detail::decompose_and_apply;

  void encode(Decomposable auto &&decomposable, ScaleEncoder auto &encoder)
    requires NoTagged<decltype(decomposable)>
  {
    decompose_and_apply(std::forward<decltype(decomposable)>(decomposable),
                        [&](auto &...args) { (encode(args, encoder), ...); });
  }


  void encode(CompactInteger auto &&integer, ScaleEncoder auto &encoder) {
    auto &&val = std::forward<decltype(untagged(integer))>(untagged(integer));
#ifdef JAM_COMPATIBILITY_ENABLED
    detail::encodeJamCompactInteger(val, encoder);
#else
    detail::encodeCompactInteger(val, encoder);
#endif
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
