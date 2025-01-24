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
    template <typename T>
    struct is_derived_of_span_impl {
      template <typename V, size_t S>
      static constexpr std::true_type test(const std::span<V, S> *);
      static constexpr std::false_type test(...);
      using type = decltype(test(std::declval<T *>()));
    };

    template <typename T>
    using is_derived_of_span = typename is_derived_of_span_impl<T>::type;

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

    template <typename T, int N = -1>
      requires std::is_aggregate_v<T>
    struct field_number_of_impl
        : std::integral_constant<
              int,
              std::conditional_t<
                  std::is_empty_v<T>,
                  std::integral_constant<int, 0>,
                  std::conditional_t<
                      is_constructible_with_n_def_args_v<T, N + 1>,
                      field_number_of_impl<T, N + 1>,
                      std::integral_constant<int, N>>>::value> {};

    template <typename T>
      requires std::is_aggregate_v<std::decay_t<T>>
    constexpr size_t field_number_of =
        field_number_of_impl<std::decay_t<T>>::value;

    constexpr size_t MAX_FIELD_NUM = 20;

    template <typename T>
    concept is_std_array =
        requires {
          typename std::remove_cvref_t<T>::value_type;
          std::tuple_size<T>::value;
        }
        and std::is_same_v<
            T,
            std::array<typename T::value_type, std::tuple_size<T>::value>>;
  }  // namespace detail

  template <typename T>
  concept SimpleCodeableAggregate =
      std::is_aggregate_v<std::decay_t<T>>  //
      and (not std::is_array_v<T>)          //
      and (not detail::is_std_array<T>)     //
      and (detail::field_number_of<T> <= detail::MAX_FIELD_NUM);

  template <typename T>
  concept SomeSpan = detail::is_derived_of_span<T>::value  //
                     and requires(T) { T::extent; };

  template <class T>
  concept HasSomeInsertMethod = requires(T v) {
    v.insert(v.end(), *v.begin());
  } or requires(T v) { v.insert_after(v.end(), *v.begin()); };

  template <class T>
  concept HasResizeMethod = requires(T v) { v.resize(v.size()); };

  template <class T>
  concept HasReserveMethod = requires(T v) { v.reserve(v.size()); };

  template <class T>
  concept HasEmplaceMethod = requires(T v) { v.emplace(*v.begin()); };

  template <class T>
  concept HasEmplaceBackMethod = requires(T v) { v.emplace_back(*v.begin()); };

  template <class T>
  concept ImplicitlyDefinedAsStatic = not(SomeSpan<T>) and  //
                                      not(HasSomeInsertMethod<T>);

  template <class T>
  concept ImplicitlyDefinedAsDynamic = not(SomeSpan<T>) and  //
                                       HasSomeInsertMethod<T>;

  template <typename T>
  concept StaticSpan = SomeSpan<T>  //
                       and (T::extent != std::dynamic_extent);

  template <typename T>
  concept DynamicSpan = SomeSpan<T>  //
                        and (T::extent == std::dynamic_extent);

  template <class T>
  concept StaticCollection = std::ranges::range<T>
                             and (ImplicitlyDefinedAsStatic<T>  //
                                  or StaticSpan<T>);

  template <class T>
  concept DynamicCollection = std::ranges::sized_range<T>
                              and (ImplicitlyDefinedAsDynamic<T>  //
                                   or DynamicSpan<T>);

  template <class T>
  concept ResizeableCollection = DynamicCollection<T>  //
                                 and HasResizeMethod<T>;

  template <class T>
  concept ExtensibleBackCollection = DynamicCollection<T>         //
                                     and not(HasResizeMethod<T>)  //
                                     and HasEmplaceBackMethod<T>;

  template <class T>
  concept RandomExtensibleCollection = DynamicCollection<T>  //
                                       and HasEmplaceMethod<T>;

}  // namespace scale
