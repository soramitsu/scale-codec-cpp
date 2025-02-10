/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <deque>
#include <memory>
#include <optional>
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

  template <typename T>
    requires std::is_integral_v<std::remove_cvref_t<T>>
  void encode(T &&v, ScaleEncoder auto &encoder)
    requires NoTagged<T>
  {
    using I = std::remove_cvref_t<T>;
    // encode bool
    if constexpr (std::is_same_v<I, bool>) {
      const uint8_t byte = (v ? 1u : 0u);
      return encoder.put(byte);
    }
    // put byte
    if constexpr (sizeof(T) == 1u) {
      // to avoid infinite recursion
      return encoder.put(static_cast<uint8_t>(v));
    }
    // encode any other integer
    encodeInteger(v, encoder);
  }

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

  void encode(FixedInteger auto &&integer, ScaleEncoder auto &encoder)
    requires NoTagged<decltype(integer)>
  {
    constexpr auto size =
        (FixedWidthIntegerTraits<decltype(integer)>::bits + 1) / 8;
    auto *bytes = new std::uint8_t[size];  // avoid useless initialization
    export_bits(integer, bytes, sizeof(uint64_t), false);
    encoder.write(std::span(bytes, size));
    delete[] bytes;
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

  void encode(OptionalBool auto &&opt_bool, ScaleEncoder auto &encoder)
    requires NoTagged<decltype(opt_bool)>
  {
    // optional bool is a special case of optional values
    // it should be encoded using one byte instead of two
    // as described in specification
    if (opt_bool.has_value()) {
      encoder.put(static_cast<uint8_t>(opt_bool.value()
                                           ? OptionalBoolEnum::OPT_TRUE
                                           : OptionalBoolEnum::OPT_FALSE));
    } else {
      encoder.put(static_cast<uint8_t>(OptionalBoolEnum::NONE));
    }
  }

  void encode(Optional auto &&optional, ScaleEncoder auto &encoder)
    requires NoTagged<decltype(optional)>
  {
    if (not optional.has_value()) {
      encoder.put(0);
    } else {
      encoder.put(1);
      encode(optional.value(), encoder);
    }
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

  template <uint8_t I>
  void encode(const Variant auto &variant, ScaleEncoder auto &encoder) {
    using T = VariantType<I, decltype(variant)>;

    if (variant_index(variant) == I) {
      encode(I, encoder);
      encode(get_variant<T>(variant), encoder);
      return;
    }

    if constexpr (std::tuple_size_v<VariantTypes<decltype(variant)>> > I + 1) {
      encode<I + 1>(variant, encoder);
    }
  }

  void encode(const Variant auto &variant, ScaleEncoder auto &encoder) {
    encode<0>(variant, encoder);
  }

  void encode(qtils::is_tagged_v auto &&tagged, ScaleEncoder auto &encoder)
    requires(not CompactInteger<decltype(tagged)>)
  {
    encode(untagged(tagged), encoder);
  }

}  // namespace scale
