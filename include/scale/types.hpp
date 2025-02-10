/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>
#include <ranges>
#include <span>
#include <tuple>
#include <type_traits>
#include <vector>

#include <boost/multiprecision/cpp_int.hpp>
#include <qtils/tagged.hpp>


#include <scale/bitvec.hpp>
#include <scale/definitions.hpp>

namespace scale {
  template <typename T>
  concept NoTagged = not qtils::is_tagged_v<T>;

  template <typename T>
  concept IsEnum = std::is_enum_v<std::remove_cvref_t<T>>;

  // Encoding components

  class EncoderBackend;

  template <typename EncoderBackendT>
    requires std::derived_from<EncoderBackendT, EncoderBackend>
  class Encoder;

  template <typename T>
  concept ScaleEncoder = requires {
    typename T::BackendType;
    requires std::derived_from<std::remove_cvref_t<T>,
                               Encoder<typename T::BackendType>>;
  };

  // Decoding components

  class DecoderBackend;

  template <typename DecoderBackendT>
    requires std::derived_from<DecoderBackendT, DecoderBackend>
  class Decoder;

  template <typename T>
  concept ScaleDecoder = requires {
    typename T::BackendType;
    requires std::derived_from<std::remove_cvref_t<T>,
                               Decoder<typename T::BackendType>>;
  };

  // Types

  using uint128_t = boost::multiprecision::uint128_t;
  using uint256_t = boost::multiprecision::uint256_t;
  using uint512_t = boost::multiprecision::uint512_t;
  using uint1024_t = boost::multiprecision::uint1024_t;

  /// @brief convenience alias for arrays of bytes
  using ByteArray = std::vector<uint8_t>;

  /// @brief convenience alias for immutable span of bytes
  using ConstSpanOfBytes = std::span<const uint8_t>;

  /// @brief convenience alias for mutable span of bytes
  using MutSpanOfBytes = std::span<uint8_t>;

  namespace detail {
    struct CompactIntegerTag;

    template <typename Backend>
    constexpr bool is_unsigned_backend = not Backend().sign();

    template <typename T>
    concept unsigned_multiprecision_integer =
        boost::multiprecision::is_unsigned_number<T>::value;

    template <typename T>
    struct is_compact_integer : std::false_type {};

    template <typename T>
    struct is_compact_integer<qtils::Tagged<T, CompactIntegerTag>>
        : std::true_type {};
  }  // namespace detail

  /// @brief Concept defining a valid Compact type
  template <typename T>
  concept CompactCompatible =
      std::unsigned_integral<std::remove_cvref_t<T>>
      or detail::unsigned_multiprecision_integer<std::remove_cvref_t<T>>;

  /// @brief Represents a compact integer value
  template <typename T>
    requires CompactCompatible<T>
  using Compact = qtils::Tagged<T, detail::CompactIntegerTag>;

  /// @brief Concept that checks if a type is a Compact integer
  template <typename T>
  concept CompactInteger =
      detail::is_compact_integer<std::remove_cvref_t<T>>::value;

  template <typename T>
  T convert_to(CompactCompatible auto &&value) {
    constexpr auto is_integral =
        std::unsigned_integral<std::remove_cvref_t<decltype(value)>>;
    if constexpr (is_integral) {
      return static_cast<T>(value);
    } else {
      return value.template convert_to<T>();
    }
  }

  template <typename T>
    requires CompactCompatible<std::remove_cvref_t<T>>
  struct CompactReflection {
   private:
    template <typename U>
    explicit CompactReflection(U &&value)
        : temp_storage(
              std::is_lvalue_reference_v<U>
                  ? std::nullopt
                  : std::optional<std::decay_t<U>>(std::forward<U>(value))),
          ref(temp_storage ? *temp_storage : value) {}

    template <typename U>
      requires CompactCompatible<std::remove_cvref_t<U>>
    friend decltype(auto) as_compact(U &&value);

   public:
    CompactReflection(const CompactReflection &) = delete;
    CompactReflection &operator=(const CompactReflection &) = delete;
    CompactReflection(CompactReflection &&) = delete;
    CompactReflection &operator=(CompactReflection &&) = delete;

    friend void encode(const CompactReflection &value,
                       ScaleEncoder auto &encoder) {
      encode(Compact<std::remove_cvref_t<T>>(value.ref), encoder);
    }
    friend void decode(const CompactReflection &value,
                       ScaleDecoder auto &decoder) {
      Compact<std::remove_cvref_t<T>> tmp;
      decode(tmp, decoder);
      value.ref = untagged(tmp);
    }

   private:
    std::optional<std::decay_t<T>> temp_storage;
    T &ref;
  };

  template <typename T>
    requires CompactCompatible<std::remove_cvref_t<T>>
  decltype(auto) as_compact(T &&value) {
    return CompactReflection<decltype(value)>(
        std::forward<decltype(value)>(value));
  }

  /// @brief OptionalBool is internal extended bool type
  enum class OptionalBoolEnum : uint8_t {
    NONE = 0u,
    OPT_TRUE = 1u,
    OPT_FALSE = 2u,
  };

  namespace detail {
    struct ArgHelper {
      template <typename T>
      operator T() const {
        return T{};
      }
    };

    template <typename T, std::size_t... Indices>
    constexpr bool is_constructible_with_n_def_args_impl(
        std::index_sequence<Indices...>) {
      return requires { T{(void(Indices), ArgHelper{})...}; };
    }

    template <typename T, size_t N>
    constexpr bool is_constructible_with_n_def_args_v =
        is_constructible_with_n_def_args_impl<T>(std::make_index_sequence<N>{});

    template <typename T, std::size_t N = 0>
    constexpr std::size_t max_constructor_args_impl() {
      if constexpr (is_constructible_with_n_def_args_v<T, N + 1>) {
        return max_constructor_args_impl<T, N + 1>();
      } else {
        return N;
      }
    }

    template <typename T>
    constexpr std::size_t max_constructor_args = max_constructor_args_impl<T>();

    template <typename T, int N = 0>
      requires std::is_aggregate_v<T>
    constexpr int field_number_of_impl() {
      if constexpr (std::is_empty_v<T>) {
        return 0;
      } else if constexpr (is_constructible_with_n_def_args_v<T, N + 1>) {
        return field_number_of_impl<T, N + 1>();
      } else {
        return N;
      }
    }

    template <typename T>
    constexpr size_t field_number_of =
        field_number_of_impl<std::remove_cvref_t<T>>();

    template <typename T>
    concept is_std_array =
        requires { typename std::tuple_size<std::remove_cvref_t<T>>::type; }
        and std::same_as<
            std::remove_cvref_t<T>,
            std::array<typename std::remove_cvref_t<T>::value_type,
                       std::tuple_size<std::remove_cvref_t<T>>::value>>;
  }  // namespace detail

  template <typename T>
  concept SomeSpan =
      std::derived_from<T, std::span<typename T::element_type, T::extent>>;

  template <typename T>
  concept HasSomeInsertMethod = requires(T v) {
    v.insert(v.end(), *v.begin());
  } or requires(T v) { v.insert_after(v.end(), *v.begin()); };

  template <typename T>
  concept HasResizeMethod =
      requires(std::remove_cvref_t<T> v) { v.resize(v.size()); };

  template <typename T>
  concept HasReserveMethod =
      requires(std::remove_cvref_t<T> v) { v.reserve(v.size()); };

  template <typename T>
  concept HasEmplaceMethod =
      requires(std::remove_cvref_t<T> v) { v.emplace(*v.begin()); };

  template <typename T>
  concept HasEmplaceBackMethod =
      requires(std::remove_cvref_t<T> v) { v.emplace_back(*v.begin()); };

  template <typename T>
  concept ImplicitlyDefinedAsStatic =
      not(SomeSpan<std::remove_cvref_t<T>>) and  //
      not(HasSomeInsertMethod<std::remove_cvref_t<T>>);

  template <typename T>
  concept ImplicitlyDefinedAsDynamic =
      not(SomeSpan<std::remove_cvref_t<T>>) and  //
      HasSomeInsertMethod<std::remove_cvref_t<T>>;

  template <typename T>
  concept StaticSpan = SomeSpan<std::remove_cvref_t<T>>  //
                       and (T::extent != std::dynamic_extent);

  template <typename T>
  concept DynamicSpan = SomeSpan<std::remove_cvref_t<T>>  //
                        and (T::extent == std::dynamic_extent);

  template <typename T>
  concept StaticCollection =
      std::ranges::range<std::remove_cvref_t<T>>
      and (ImplicitlyDefinedAsStatic<std::remove_cvref_t<T>>
           or StaticSpan<std::remove_cvref_t<T>>);

  template <typename T>
  concept DynamicCollection =
      std::ranges::sized_range<std::remove_cvref_t<T>>
      and (ImplicitlyDefinedAsDynamic<std::remove_cvref_t<T>>
           or DynamicSpan<std::remove_cvref_t<T>>);

  template <typename T>
  concept ResizeableCollection = DynamicCollection<std::remove_cvref_t<T>>
                                 and HasResizeMethod<std::remove_cvref_t<T>>;

  template <typename T>
  concept ExtensibleBackCollection =
      DynamicCollection<std::remove_cvref_t<T>>
      and not(HasResizeMethod<std::remove_cvref_t<T>>)
      and HasEmplaceBackMethod<std::remove_cvref_t<T>>;

  template <typename T>
  concept RandomExtensibleCollection =
      DynamicCollection<std::remove_cvref_t<T>>
      and HasEmplaceMethod<std::remove_cvref_t<T>>;

  template <typename T>
  concept OptionalBool =
      std::same_as<std::remove_cvref_t<T>, std::optional<bool>>;

  template <typename T>
  concept Optional =
      requires { typename std::remove_cvref_t<T>::value_type; }
      and (std::is_same_v<
           std::remove_cvref_t<T>,
           std::optional<typename std::remove_cvref_t<T>::value_type>>)
      and (not OptionalBool<T>);



  namespace {
    template <typename T>
    concept HasCustomDecomposition = requires(T &obj, const T &cobj) {
      {
        obj._custom_decompose_and_apply([](auto &&...) {})
      };
      {
        cobj._custom_decompose_and_apply([](auto &&...) {})
      };
    };

    template <typename T>
    struct HasDecomposeAndApplyHelper {
      template <typename U>
      static auto test(int) -> decltype(decompose_and_apply(std::declval<U &>(),
                                                            [](auto &&...) {}),
                                        std::true_type{});

      template <typename>
      static auto test(...) -> std::false_type;

      static constexpr bool value = decltype(test<T>(0))::value;
    };

    template <typename T>
    concept HasDecomposeAndApply = HasDecomposeAndApplyHelper<T>::value;

    template <typename T>
    concept CustomDecomposable =
        HasCustomDecomposition<T> && HasDecomposeAndApply<T>;
  }  // namespace

  namespace {
    template <typename T, std::size_t N = detail::MAX_FIELD_NUM>
    concept Array =
        std::is_bounded_array_v<T>
        or (std::same_as<
            std::remove_cvref_t<T>,
            std::array<typename std::remove_cvref_t<T>::value_type,
                       std::tuple_size<std::remove_cvref_t<T>>::value>>);

    template <typename T>
    struct array_size_impl;

    template <typename T, std::size_t N>
    struct array_size_impl<std::array<T, N>> {
      static constexpr std::size_t value = N;
    };

    template <typename T, std::size_t N>
    struct array_size_impl<T[N]> {
      static constexpr std::size_t value = N;
    };

    template <typename T>
    inline constexpr std::size_t array_size =
        array_size_impl<std::remove_cvref_t<T>>::value;

    template <typename T, std::size_t N = detail::MAX_FIELD_NUM>
    concept ArrayWithMaxSize = Array<T> and array_size<T> <= N;

    template <typename T>
    concept DecomposableArray =
        not CustomDecomposable<T> and ArrayWithMaxSize<T>;
  }  // namespace

  namespace {
    template <typename T, typename = void>
    struct is_structurally_bindable : std::false_type {};

    template <typename T>
    struct is_structurally_bindable<
        T,
        std::void_t<decltype(std::tuple_size<T>::value)>> : std::true_type {};

    template <typename T, typename = void>
    struct structured_binding_size : std::integral_constant<std::size_t, 0> {};

    template <typename T>
    struct structured_binding_size<
        T,
        std::void_t<decltype(std::tuple_size<T>::value)>> : std::tuple_size<T> {
    };

    template <typename T>
    constexpr std::size_t structured_binding_size_v =
        structured_binding_size<std::remove_cvref_t<T>>::value;

    template <typename T>
    concept StructurallyBindable =
        is_structurally_bindable<std::remove_cvref_t<T>>::value;

    template <typename T>
    concept StructurallyBindableWithMaxSize =
        StructurallyBindable<T>
        and structured_binding_size_v<std::remove_cvref_t<T>>
                <= detail::MAX_FIELD_NUM;

    template <typename T>
    concept DecomposableStructurallyBindable =
        (not CustomDecomposable<T>) and (not DecomposableArray<T>)
        and StructurallyBindableWithMaxSize<T>;
  }  // namespace

  namespace {
    template <typename T>
    concept Aggregate = std::is_aggregate_v<std::remove_cvref_t<T>>;

    template <typename T>
    concept AggregateWithMaxSize =
        Aggregate<T> and (detail::field_number_of<T> <= detail::MAX_FIELD_NUM);

    template <typename T>
    concept DecomposableAggregate =
        (not CustomDecomposable<T>)                    //
        and(not DecomposableArray<T>)                  //
        and (not DecomposableStructurallyBindable<T>)  //
        and (not DynamicCollection<T>)                 //
        and (not StaticCollection<T>)                  //
        and (not std::same_as<T, BitVec>)              //
        and AggregateWithMaxSize<T>;
  }  // namespace

  template <typename T>
  concept Decomposable =
      CustomDecomposable<std::remove_cvref_t<T>>
      or DecomposableArray<std::remove_cvref_t<T>>
      or DecomposableStructurallyBindable<std::remove_cvref_t<T>>
      or DecomposableAggregate<std::remove_cvref_t<T>>;

}  // namespace scale
