/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <iterator>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include <boost/endian/buffers.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <qtils/tagged.hpp>

#include <scale/bitvec.hpp>
// ReSharper disable once CppUnusedIncludeDirective
#include <scale/definitions.hpp>
#include <scale/detail/fixed_width_integer.hpp>
#ifdef JAM_COMPATIBILITY_ENABLED
#include <scale/detail/jam_compact_integer.hpp>
#else
#include <scale/detail/compact_integer.hpp>
#endif

#include <qtils/outcome.hpp>

#include <scale/backend/from_bytes.hpp>
#include <scale/configurable.hpp>
#include <scale/detail/decompose_and_apply.hpp>
#include <scale/enum_traits.hpp>
#include <scale/scale_error.hpp>

namespace scale {

  template <typename DecoderBackendT>
    requires std::derived_from<DecoderBackendT, DecoderBackend>
  class Decoder : public Configurable {
   public:
    using BackendType = DecoderBackendT;

    struct Z {};

    static constexpr auto N = detail::max_constructor_args<BackendType>;

    template <std::size_t... I1, std::size_t... I2, typename... Args>
    explicit Decoder(Z,
                     std::index_sequence<I1...>,
                     std::index_sequence<I2...>,
                     Args &&...args)
        : Configurable(std::get<I2>(std::forward_as_tuple(args...))...),
          backend_(std::get<I1>(std::forward_as_tuple(args...))...) {
      std::cout << "Decoder constructed\n";
    }

    template <std::size_t N, typename... Args>
    explicit Decoder(Args &&...args)
        : Decoder(Z{},
                  std::make_index_sequence<sizeof...(Args) - N>{},
                  std::make_index_sequence<N>{},
                  std::forward<Args>(args)...) {}

    auto &backend() {
      return backend_;
    }

#ifdef CUSTOM_CONFIG_ENABLED
    explicit Decoder(ConstSpanOfBytes data, const MaybeConfig auto &...configs)
        : Configurable(configs...), backend_(data) {}
#else
    // [[deprecated("Scale has compiled without custom config support")]]  //
    // Decoder(ConstSpanOfBytes data,
    //                    const MaybeConfig auto &...configs) = delete;

    template <typename... Args>
    explicit Decoder(Args &&...args) : backend_(std::forward<Args>(args)...) {}
#endif

    template <typename T>
    Decoder &operator>>(T &&value) {
      decode(std::forward<T>(value), *this);
      return *this;
    }

    /**
     * @brief hasMore Checks whether n more bytes are available
     * @param n Number of bytes to check
     * @return True if n more bytes are available and false otherwise
     */
    [[nodiscard]] bool has(uint64_t n) const {
      return backend_.has(n);
    }

    /**
     * @brief takes one byte from backend
     */
    uint8_t take() {
      if (not backend_.has(1)) {
        raise(DecodeError::NOT_ENOUGH_DATA);
      }
      return backend_.take();
    }

    void read(std::span<uint8_t> out) {
      if (not backend_.has(out.size())) {
        raise(DecodeError::NOT_ENOUGH_DATA);
      }
      return backend_.read(out);
    }

   private:
    BackendType backend_;
  };

  template <typename T>
    requires std::is_default_constructible_v<std::remove_const_t<T>>
  void decode(std::shared_ptr<T> &v, ScaleDecoder auto &decoder) {
    v = std::make_shared<std::remove_const_t<T>>();
    decode(const_cast<std::remove_const_t<T> &>(*v), decoder);
  }

  template <typename T>
    requires std::is_default_constructible_v<std::remove_const_t<T>>
  void decode(std::unique_ptr<T> &v, ScaleDecoder auto &decoder) {
    v = std::make_unique<std::remove_const_t<T>>();
    decode(const_cast<std::remove_const_t<T> &>(*v), decoder);
  }

  template <typename T>
    requires std::is_default_constructible_v<std::remove_const_t<T>>
  void decode(std::optional<T> &v, ScaleDecoder auto &decoder) {
    // Special case for `std::optional<bool>`
    if constexpr (std::is_same_v<std::remove_const_t<T>, bool>) {
      auto byte = decoder.take();
      switch (static_cast<OptionalBoolEnum>(byte)) {
        case OptionalBoolEnum::NONE:
          v.reset();
          return;
        case OptionalBoolEnum::OPT_FALSE:
          v = false;
          return;
        case OptionalBoolEnum::OPT_TRUE:
          v = true;
          return;
        default:
          raise(DecodeError::UNEXPECTED_VALUE);
      }
    }

    // Check if the optional contains a value
    bool has_value = false;
    decode(has_value, decoder);

    if (not has_value) {
      v.reset();  // Reset the optional if it has no value
      return;
    }

    // Decode the value
    v.emplace();  // Initialize the object inside the optional
    decode(const_cast<std::remove_const_t<T> &>(v.value()), decoder);  // NOLINT
  }

  template <typename T>
    requires std::is_integral_v<std::remove_cvref_t<T>>
  void decode(T &v, ScaleDecoder auto &decoder) {
    using I = std::decay_t<T>;
    // check bool
    if constexpr (std::is_same_v<I, bool>) {
      switch (auto byte = decoder.take()) {
        case 0u:
          v = false;
          return;
        case 1u:
          v = true;
          return;
        default:
          raise(DecodeError::UNEXPECTED_VALUE);
      }
    }
    // check byte
    if constexpr (sizeof(T) == 1u) {
      v = decoder.take();
      return;
    }
    // decode any other integer
    if (not decoder.has(sizeof(T))) {
      raise(DecodeError::NOT_ENOUGH_DATA);
    }
    std::array<uint8_t, sizeof(T)> data;
    for (auto &i : data) {
      i = decoder.take();
    }

    v = boost::endian::endian_load<T, sizeof(T), boost::endian::order::little>(
        data.data());
  }

  void decode(FixedInteger auto &integer, ScaleDecoder auto &decoder) {
    constexpr auto size =
        (FixedWidthIntegerTraits<decltype(integer)>::bits + 1) / 8;
    auto *bytes = new std::uint8_t[size];  // avoid useless initialization
    decoder.read(std::span(bytes, size));
    import_bits(integer, bytes, bytes + size, sizeof(uint64_t), false);
    delete[] bytes;
  }

  void decode(std::vector<bool> &collection, ScaleDecoder auto &decoder) {
    size_t item_count;
    decode(as_compact(item_count), decoder);
    if (item_count > collection.max_size()) {
      raise(DecodeError::TOO_MANY_ITEMS);
    }

    try {
      collection.resize(item_count);
    } catch (const std::bad_alloc &) {
      raise(DecodeError::TOO_MANY_ITEMS);
    }

    for (size_t i = 0u; i < item_count; ++i) {
      bool item;
      decode(item, decoder);
      collection[i] = item;
    }
  }

  void decode(BitVec &v, ScaleDecoder auto &decoder) {
    size_t size;
    decode(as_compact(size), decoder);
    if (not decoder.has((size + 7) / 8)) {
      raise(DecodeError::NOT_ENOUGH_DATA);
    }
    v.bits.resize(size);
    size_t i = 0;
    uint8_t byte = 0;
    for (std::vector<bool>::reference bit : v.bits) {
      if (i % 8 == 0) {
        byte = decoder.take();
      }
      bit.operator=(((byte >> (i % 8)) & 1) != 0);
      ++i;
    }
  }

  /// @note Implementation prohibited as potentially dangerous.
  /// Use manual decoding instead
  void decode(DynamicSpan auto &collection,
              ScaleDecoder auto &decoder) = delete;

  void decode(StaticCollection auto &collection, ScaleDecoder auto &decoder)
    requires(not Decomposable<decltype(collection)>)
            and (not qtils::is_tagged_v<decltype(collection)>)
  {
    for (auto &item : collection) {
      decode(item, decoder);
    }
  }

  void decode(ExtensibleBackCollection auto &collection,
              ScaleDecoder auto &decoder)
    requires(not qtils::is_tagged_v<decltype(collection)>)
  {
    using size_type = typename std::decay_t<decltype(collection)>::size_type;

    size_t item_count;
    decode(as_compact(item_count), decoder);
    if (item_count > collection.max_size()) {
      raise(DecodeError::TOO_MANY_ITEMS);
    }

    collection.clear();
    try {
      collection.reserve(item_count);
    } catch (const std::bad_alloc &) {
      raise(DecodeError::TOO_MANY_ITEMS);
    }

    for (size_type i = 0u; i < item_count; ++i) {
      collection.emplace_back();
      decode(collection.back(), decoder);
    }
  }

  void decode(ResizeableCollection auto &collection, ScaleDecoder auto &decoder)
    requires NoTagged<decltype(collection)>
  {
    size_t item_count;
    decode(as_compact(item_count), decoder);
    if (item_count > collection.max_size()) {
      raise(DecodeError::TOO_MANY_ITEMS);
    }

    try {
      collection.resize(item_count);
    } catch (const std::bad_alloc &) {
      raise(DecodeError::TOO_MANY_ITEMS);
    }

    for (auto &item : collection) {
      decode(item, decoder);
    }
  }

  /**
   * @brief scale-decodes to non-sequential collection (which can not be
   * reserved space or resize, but each element can be emplaced while
   * decoding)
   */
  void decode(RandomExtensibleCollection auto &collection,
              ScaleDecoder auto &decoder)
    requires(not qtils::is_tagged_v<decltype(collection)>)
  {
    using size_type = typename std::decay_t<decltype(collection)>::size_type;
    using value_type = typename std::decay_t<decltype(collection)>::value_type;

    size_type item_count;
    decode(as_compact(item_count), decoder);
    if (item_count > collection.max_size()) {
      raise(DecodeError::TOO_MANY_ITEMS);
    }

    collection.clear();
    for (size_type i = 0u; i < item_count; ++i) {
      value_type item;
      decode(item, decoder);
      try {
        collection.emplace(std::move(item));
      } catch (const std::bad_alloc &) {
        raise(DecodeError::TOO_MANY_ITEMS);
      }
    }
  }

  void decode(IsEnum auto &v, ScaleDecoder auto &decoder)
    requires NoTagged<decltype(v)>
  {
    using E = std::decay_t<decltype(v)>;
    std::underlying_type_t<E> value;
    decoder >> value;
    if (is_valid_enum_value<E>(value)) {
      v = static_cast<E>(value);
      return;
    }
    raise(DecodeError::INVALID_ENUM_VALUE);
  }

  using detail::decompose_and_apply;

  void decode(Decomposable auto &decomposable, ScaleDecoder auto &decoder)
    requires NoTagged<decltype(decomposable)>
  {
    return decompose_and_apply(decomposable, [&](auto &...args) {
      (decode(const_cast<std::remove_cvref_t<decltype(args)> &>(args), decoder),
       ...);
    });
  }

  void decode(CompactInteger auto &v, ScaleDecoder auto &decoder) {
    using Integer = qtils::untagged_t<std::remove_cvref_t<decltype(v)>>;
    auto &&integer =
#ifdef JAM_COMPATIBILITY_ENABLED
        detail::decodeJamCompactInteger(decoder);
#else
        detail::decodeCompactInteger(decoder);
#endif
    if constexpr (std::is_integral_v<Integer>) {
      if (integer > std::numeric_limits<Integer>::max()) {
        raise(DecodeError::DECODED_VALUE_OVERFLOWS_TARGET);
      }
    } else {
      if (not integer.is_zero()
          and msb(integer) >= std::numeric_limits<Integer>::digits) {
        raise(DecodeError::DECODED_VALUE_OVERFLOWS_TARGET);
      }
    }
    v = convert_to<Integer>(integer);
  }

  template <size_t I>
  void decode(Variant auto &variant,
              ScaleDecoder auto &decoder,
              const size_t index)
    requires(I < std::tuple_size_v<VariantTypes<decltype(variant)>>)
  {
    using T = VariantType<I, decltype(variant)>;
    static_assert(std::is_default_constructible_v<T>,
                  "All types of variant must be default constructible");
    if (I == index) {
      T value;
      decode(value, decoder);
      variant = std::forward<T>(value);
      return;
    }
    if constexpr (std::tuple_size_v<VariantTypes<decltype(variant)>> > I + 1) {
      decode<I + 1>(variant, decoder, index);
    }
  }

  void decode(Variant auto &variant, ScaleDecoder auto &decoder) {
    uint8_t index = decoder.take();
    if (index < std::tuple_size_v<VariantTypes<decltype(variant)>>) {
      return decode<0>(variant, decoder, index);
    }
    raise(DecodeError::WRONG_TYPE_INDEX);
  }

  void decode(qtils::is_tagged_v auto &tagged, ScaleDecoder auto &decoder)
    requires(not CompactInteger<decltype(tagged)>)
  {
    decode(untagged(tagged), decoder);
  }

}  // namespace scale