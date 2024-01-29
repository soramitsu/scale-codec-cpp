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

  template <typename T>
  struct __is_derived_of_span_impl {
    template <typename V, size_t S>
    static constexpr std::true_type test(const std::span<V, S> *);
    static constexpr std::false_type test(...);
    using type = decltype(test(std::declval<T *>()));
  };

  template <typename T>
  using __is_derived_of_span = typename __is_derived_of_span_impl<T>::type;

  template <typename T>
  concept SomeSpan = __is_derived_of_span<T>::value  //
                     and requires(T) { T::extent; };

  template <class T>
  concept HasSomeInsertMethod =
      requires(T v) { v.insert(v.end(), *v.begin()); }
      or requires(T v) { v.insert_after(v.end(), *v.begin()); };

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

namespace scale::compact {

  /**
   * @brief categories of compact encoding
   */
  struct EncodingCategoryLimits {
    // min integer encoded by 2 bytes
    constexpr static size_t kMinUint16 = (1ul << 6u);
    // min integer encoded by 4 bytes
    constexpr static size_t kMinUint32 = (1ul << 14u);
    // min integer encoded as multibyte
    constexpr static size_t kMinBigInteger = (1ul << 30u);
  };

}  // namespace scale::compact
