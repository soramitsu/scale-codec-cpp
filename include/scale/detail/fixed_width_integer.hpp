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

  template <typename T>
  concept BigFixedWidthInteger =
      std::is_same_v<std::remove_cvref_t<T>, uint128_t>
      or std::is_same_v<std::remove_cvref_t<T>, uint256_t>
      or std::is_same_v<std::remove_cvref_t<T>, uint512_t>
      or std::is_same_v<std::remove_cvref_t<T>, uint1024_t>;

  template <typename T, typename = void>
  struct FixedWidthIntegerTraits;

  // Для целочисленных типов
  template <std::integral T>
  struct FixedWidthIntegerTraits<T> {
    static constexpr size_t bytes = sizeof(T);
    static constexpr size_t bits = bytes * 8;
  };

  template <typename T>
  struct FixedWidthIntegerTraits<T, std::enable_if_t<std::is_class_v<T>>> {
   private:
    static constexpr size_t compute_bytes() {
      T temp{};
      return temp.backend().size() * sizeof(temp.backend().limbs()[0]);
    }

   public:
    static constexpr size_t bytes = compute_bytes();
    static constexpr size_t bits = bytes * 8;
  };

  namespace detail {
    template <typename To, typename From>
      requires std::is_convertible_v<From, To>
    To convert_to(From value) {
      return static_cast<To>(value);
    }

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
   * encodeInteger encodes any integer type to little-endian representation
   * @tparam T integer type
   * @param value integer value
   * @param stream output stream
   */
  template <typename T>
    requires std::is_integral_v<std::remove_cvref_t<T>>
  void encodeInteger(T value, ScaleEncoderStream &stream) {
    using I = std::remove_cvref_t<T>;
    constexpr size_t size = sizeof(I);
    constexpr size_t bits = size * 8;
    boost::endian::endian_buffer<boost::endian::order::little, I, bits> buf{};
    buf = value;  // cannot initialize, only assign
    for (size_t i = 0; i < size; ++i) {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      stream << buf.data()[i];
    }
  }

  template <typename S>
    requires std::derived_from<std::remove_cvref_t<S>, ScaleEncoderStream>
  void encodeInteger(const BigFixedWidthInteger auto &v, S &s) {
    using Integer = std::remove_cvref_t<decltype(v)>;
    static constexpr auto bits = FixedWidthIntegerTraits<Integer>::bits;
    for (size_t i = 0; i < bits; i += 8) {
      s.putByte(detail::convert_to<uint8_t>((v >> i) & 0xFFu));
    }
  }

  template <typename T>
    requires std::is_integral_v<std::remove_cvref_t<T>>
  void decodeInteger(T &value, ScaleDecoderStream &stream) {
    using I = std::remove_cvref_t<T>;
    constexpr size_t size = sizeof(I);
    constexpr size_t bits = size * 8;
    boost::endian::endian_buffer<boost::endian::order::little, I, bits> buf{};
    buf = value;  // cannot initialize, only assign
    for (size_t i = 0; i < size; ++i) {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      stream >> buf.data()[i];
    }
  }

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
