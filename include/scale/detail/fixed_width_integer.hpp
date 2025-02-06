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
  class ScaleEncoderStream;
  class ScaleDecoderStream;
}  // namespace scale

namespace scale {

  /**
   * @brief Concept for big fixed-width integer types.
   *
   * This concept checks if the given type is one of the predefined large
   * integer types.
   */
  template <typename T>
  concept BigFixedWidthInteger =
      std::is_same_v<std::remove_cvref_t<T>, uint128_t>
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
  struct FixedWidthIntegerTraits<T, std::enable_if_t<std::is_class_v<T>>> {
   private:
    // Compute count significant bytes needed to store value,
    // except any metadata and auxiliaries
    static constexpr size_t compute_bytes() {
      T temp{};
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
      } catch (const std::runtime_error &e) {
        // scale::decode catches std::system_errors
        throw std::system_error{
            make_error_code(std::errc::value_too_large),
            "This integer conversion would lead to information loss"};
      }
    }
  }  // namespace detail

  /**
   * @brief Encodes an integer to little-endian representation.
   * @tparam T Integer type.
   * @param value Integer value to encode.
   * @param stream Output stream where encoded data is written.
   */
  template <typename T>
    requires std::is_integral_v<std::remove_cvref_t<T>>
  void encodeInteger(T value, ScaleEncoderStream &stream) {
    using I = std::remove_cvref_t<T>;
    constexpr size_t size = sizeof(I);
    constexpr size_t bits = size * 8;
    boost::endian::endian_buffer<boost::endian::order::little, I, bits> buf{};
    buf = value;  // Assign value to endian buffer
    for (size_t i = 0; i < size; ++i) {
      stream << buf.data()[i];  // Write each byte to the stream
    }
  }

  /**
   * @brief Encodes large fixed-width integers to little-endian format.
   * @tparam S Type of the SCALE encoder stream.
   * @param v Integer value to encode.
   * @param s Encoder stream.
   */
  template <typename S>
    requires std::derived_from<std::remove_cvref_t<S>, ScaleEncoderStream>
  void encodeInteger(const BigFixedWidthInteger auto &v, S &s) {
    using Integer = std::remove_cvref_t<decltype(v)>;
    static constexpr auto bits = FixedWidthIntegerTraits<Integer>::bits;
    for (size_t i = 0; i < bits; i += 8) {
      s.putByte(detail::convert_to<uint8_t>((v >> i) & 0xFFu));
    }
  }

  /**
   * @brief Decodes an integer from little-endian representation.
   * @tparam T Integer type.
   * @param value Reference to store the decoded integer.
   * @param stream Input stream from which data is read.
   */
  template <typename T>
    requires std::is_integral_v<std::remove_cvref_t<T>>
  void decodeInteger(T &value, ScaleDecoderStream &stream) {
    using I = std::remove_cvref_t<T>;
    constexpr size_t size = sizeof(I);
    constexpr size_t bits = size * 8;
    boost::endian::endian_buffer<boost::endian::order::little, I, bits> buf{};
    buf = value;  // Assign initial value
    for (size_t i = 0; i < size; ++i) {
      stream >> buf.data()[i];  // Read each byte from the stream
    }
  }

  /**
   * @brief Decodes large fixed-width integers from little-endian format.
   * @tparam S Type of the SCALE decoder stream.
   * @param v Reference to store the decoded integer.
   * @param s Decoder stream.
   */
  template <typename S>
    requires std::derived_from<std::remove_cvref_t<S>, ScaleDecoderStream>
  void decodeInteger(BigFixedWidthInteger auto &v, S &s) {
    using Integer = std::remove_cvref_t<decltype(v)>;
    static constexpr auto bits = FixedWidthIntegerTraits<Integer>::bits;

    v = 0;
    for (size_t i = 0; i < bits; i += 8) {
      uint8_t byte;
      s >> byte;
      v |= Integer(byte) << i;
    }
  }

}  // namespace scale
