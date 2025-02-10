/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <type_traits>

#include <boost/endian/buffers.hpp>
#include <boost/multiprecision/cpp_int.hpp>

#include <scale/types.hpp>

namespace scale {

  /**
   * @brief Concept for big fixed-width integer types.
   *
   * This concept checks if the given type is one of the predefined large
   * integer types.
   */
  template <typename T>
  concept FixedInteger = std::is_same_v<std::remove_cvref_t<T>, uint128_t>
                         or std::is_same_v<std::remove_cvref_t<T>, uint256_t>
                         or std::is_same_v<std::remove_cvref_t<T>, uint512_t>
                         or std::is_same_v<std::remove_cvref_t<T>, uint1024_t>;

  /**
   * @brief Traits for determining the size of fixed-width integer types.
   *
   * This primary template handles standard integral types.
   */
  template <typename T, typename = void>
  struct FixedWidthIntegerTraits;

  /**
   * @brief Specialization for built-in integral types.
   */
  template <std::integral T>
  struct FixedWidthIntegerTraits<T> {
    static constexpr size_t bytes = sizeof(T);
    static constexpr size_t bits = bytes * 8;
  };

  /**
   * @brief Specialization for Boost multiprecision integer types.
   *
   * Computes the number of bytes required to store the value,
   * excluding any metadata or auxiliary data.
   */
  template <typename T>
  struct FixedWidthIntegerTraits<
      T,
      std::enable_if_t<std::is_class_v<std::remove_cvref_t<T>>>> {
   private:
    // Compute count significant bytes needed to store value,
    // except any metadata and auxiliaries
    static constexpr size_t compute_bytes() {
      std::remove_cvref_t<T> temp{};
      return temp.backend().size() * sizeof(temp.backend().limbs()[0]);
    }

   public:
    static constexpr size_t bytes = compute_bytes();
    static constexpr size_t bits = bytes * 8;
  };

  namespace detail {
    /**
     * @brief Converts a value from one type to another using static_cast.
     * @tparam To Target type.
     * @tparam From Source type.
     * @param value Value to convert.
     * @return Converted value.
     */
    template <typename To, typename From>
      requires std::is_convertible_v<From, To>
    To convert_to(From value) {
      return static_cast<To>(value);
    }

    /**
     * @brief Specialized conversion for Boost multiprecision numbers.
     * @tparam To Target type.
     * @tparam From Boost multiprecision number type.
     * @param value Value to convert.
     * @return Converted value.
     * @throws std::system_error if conversion results in data loss.
     */
    template <typename To, typename From>
      requires boost::multiprecision::is_number<From>::value
    To convert_to(const From &value) {
      try {
        return value.template convert_to<To>();
      } catch (const std::runtime_error &) {
        // scale::decode catches std::system_errors
        throw std::system_error{
            make_error_code(std::errc::value_too_large),
            "This integer conversion would lead to information loss"};
      }
    }
  }  // namespace detail

  /**
   * @brief Encodes an integer to little-endian representation.
   * @param value Integer value to encode.
   */
  template <typename T>
    requires std::is_integral_v<std::remove_cvref_t<T>>
  void encodeInteger(T value, ScaleEncoder auto &encoder) {
    using I = std::remove_cvref_t<T>;
    constexpr size_t size = sizeof(I);
    constexpr size_t bits = size * 8;
    boost::endian::endian_buffer<boost::endian::order::little, I, bits> buf;
    buf = value;  // Assign value to endian buffer
    encoder.write({buf.data(), size});  // Write each byte to the backend
  }

}  // namespace scale
