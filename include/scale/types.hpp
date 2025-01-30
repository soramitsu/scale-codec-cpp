/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>
#include <ranges>
#include <span>
#include <vector>

#include <boost/multiprecision/cpp_int.hpp>

namespace scale {

  /// @brief convenience alias for arrays of bytes
  using ByteArray = std::vector<uint8_t>;

  /// @brief convenience alias for immutable span of bytes
  using ConstSpanOfBytes = std::span<const uint8_t>;

  /// @brief convenience alias for mutable span of bytes
  using MutSpanOfBytes = std::span<uint8_t>;

  /// @brief represents compact integer value
  using CompactInteger = boost::multiprecision::cpp_int;

  /// @brief OptionalBool is internal extended bool type
  enum class OptionalBool : uint8_t {
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
      requires std::is_aggregate_v<T>
    constexpr bool is_constructible_with_n_def_args_impl(
        std::index_sequence<Indices...>) {
      return std::is_constructible_v<T,
                                     decltype((void(Indices), ArgHelper{}))...>;
    }

    template <typename T, size_t N>
    constexpr bool is_constructible_with_n_def_args_v =
        is_constructible_with_n_def_args_impl<T>(std::make_index_sequence<N>{});

    template <typename T, int N = 0>
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

    constexpr size_t MAX_FIELD_NUM = 20;

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
      std::is_base_of_v<std::span<typename T::element_type, T::extent>, T>;

  template <typename T>
  concept HasSomeInsertMethod = requires(T v) {
    v.insert(v.end(), *v.begin());
  } or requires(T v) { v.insert_after(v.end(), *v.begin()); };

  template <typename T>
  concept HasResizeMethod = requires(T v) { v.resize(v.size()); };

  template <typename T>
  concept HasReserveMethod = requires(T v) { v.reserve(v.size()); };

  template <typename T>
  concept HasEmplaceMethod = requires(T v) { v.emplace(*v.begin()); };

  template <typename T>
  concept HasEmplaceBackMethod = requires(T v) { v.emplace_back(*v.begin()); };

  template <typename T>
  concept ImplicitlyDefinedAsStatic = not(SomeSpan<T>) and  //
                                      not(HasSomeInsertMethod<T>);

  template <typename T>
  concept ImplicitlyDefinedAsDynamic = not(SomeSpan<T>) and  //
                                       HasSomeInsertMethod<T>;

  template <typename T>
  concept StaticSpan = SomeSpan<T>  //
                       and (T::extent != std::dynamic_extent);

  template <typename T>
  concept DynamicSpan = SomeSpan<T>  //
                        and (T::extent == std::dynamic_extent);

  template <typename T>
  concept StaticCollection = std::ranges::range<T>
                             and (ImplicitlyDefinedAsStatic<T>  //
                                  or StaticSpan<T>);

  template <typename T>
  concept DynamicCollection = std::ranges::sized_range<T>
                              and (ImplicitlyDefinedAsDynamic<T>  //
                                   or DynamicSpan<T>);

  template <typename T>
  concept ResizeableCollection = DynamicCollection<T>  //
                                 and HasResizeMethod<T>;

  template <typename T>
  concept ExtensibleBackCollection = DynamicCollection<T>         //
                                     and not(HasResizeMethod<T>)  //
                                     and HasEmplaceBackMethod<T>;

  template <typename T>
  concept RandomExtensibleCollection = DynamicCollection<T>  //
                                       and HasEmplaceMethod<T>;

  template <typename T>
concept SimpleCodeableAggregate =
    std::is_aggregate_v<std::remove_cvref_t<T>>  //
    and (not DynamicCollection<T>)                 //
    and (not std::is_array_v<T>)                 //
    and (not detail::is_std_array<T>)            //
    and (detail::field_number_of<T> <= detail::MAX_FIELD_NUM);

  template <typename T, typename = void>
  struct HasDecomposeAndApply : std::false_type {};

  template <typename T>
  struct HasDecomposeAndApply<T,
                              std::void_t<decltype(decompose_and_apply(
                                  std::declval<T>(), [](auto &&...) {}))>>
      : std::true_type {};

  template <typename T>
  concept CustomDecomposable =
      not SimpleCodeableAggregate<T> and HasDecomposeAndApply<T>::value;


}  // namespace scale
