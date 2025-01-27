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

#ifdef __has_include
#if __has_include(<boost/variant.hpp>)
#include <boost/variant.hpp>
#define USE_BOOST_VARIANT
#endif
#endif

#include <scale/bitvec.hpp>
#include <scale/definitions.hpp>
#include <scale/detail/aggregate.hpp>
#include <scale/detail/fixed_width_integer.hpp>
#ifdef JAM_COMPATIBILITY_ENABLED
#include <scale/detail/jam_compact_integer.hpp>
#else
#include <scale/detail/compact_integer.hpp>
#endif
#include <scale/configurable.hpp>
#include <scale/scale_error.hpp>
#include <scale/types.hpp>

namespace scale {

  /**
   * @class ScaleEncoderStream designed to scale-encode data to stream
   */
  class ScaleEncoderStream : public Configurable {
   public:
    // special tag to differentiate encoding streams from others
    static constexpr auto is_encoder_stream = true;

    ScaleEncoderStream();

    /**
     * Stream initialization
     * @param drop_data - when true will only count encoded data size while
     * omitting the data itself
     */
    explicit ScaleEncoderStream(bool drop_data);

#ifdef CUSTOM_CONFIG_ENABLED
    explicit ScaleEncoderStream(const MaybeConfig auto &...configs)
        : Configurable(configs...) {}

    explicit ScaleEncoderStream(bool drop_data,
                                const MaybeConfig auto &...configs)
        : Configurable(configs...), drop_data_(drop_data) {}
#else
    [[deprecated("Scale has compiled without custom config support")]]  //
    explicit ScaleEncoderStream(const MaybeConfig auto &...configs) = delete;

    [[deprecated("Scale has compiled without custom config support")]]  //
    explicit ScaleEncoderStream(bool drop_data,
                                const MaybeConfig auto &...configs) = delete;
#endif

    /**
     * @return vector of bytes containing encoded data
     */
    [[nodiscard]] std::vector<uint8_t> to_vector() const;

    /**
     * Get amount of encoded data written to the stream
     * @return size in bytes
     */
    [[nodiscard]] size_t size() const;

    [[nodiscard]] auto begin() const {
      return stream_.begin();
    }
    [[nodiscard]] auto end() const {
      return stream_.end();
    }

    /**
     * @brief scale-encodes aggregate
     * @param v aggregate to encode
     * @return reference to stream
     */
    ScaleEncoderStream &operator<<(const SimpleCodeableAggregate auto &v) {
      return detail::as_decomposed(
          v, [&](const auto &...args) -> ScaleEncoderStream & {
            return (*this << ... << args);
          });
    }

    /**
     * @brief scale-encodes range
     * @param collection range to encode
     * @return reference to stream
     */
    ScaleEncoderStream &operator<<(const DynamicCollection auto &collection) {
      return encodeDynamicCollection(collection);
    }

    ScaleEncoderStream &operator<<(const StaticCollection auto &collection) {
      return encodeStaticCollection(collection);
    }

    /**
     * @brief scale-encodes BitVec
     */
    ScaleEncoderStream &operator<<(const BitVec &v);

    /**
     * @brief scale-encodes pair of values
     * @tparam F first value type
     * @tparam S second value type
     * @param p pair of values to encode
     * @return reference to stream
     */
    template <class F, class S>
    ScaleEncoderStream &operator<<(const std::pair<F, S> &p) {
      return *this << p.first << p.second;
    }

    /**
     * @brief scale-encodes tuple
     * @tparam T enumeration of types
     * @param v tuple
     * @return reference to stream
     */
    template <class... Ts>
    ScaleEncoderStream &operator<<(const std::tuple<Ts...> &v) {
      if constexpr (sizeof...(Ts) > 0) {
        encodeElementOfTuple<0>(v);
      }
      return *this;
    }

    /**
     * @brief scale-encodes variant value
     * @tparam T type list
     * @param v value to encode
     * @return reference to stream
     */
    template <class... T>
    ScaleEncoderStream &operator<<(const std::variant<T...> &v) {
      tryEncodeAsOneOfVariant<0>(v);
      return *this;
    }

#ifdef USE_BOOST_VARIANT
    /**
     * @brief scale-encodes variant value
     * @tparam T type list
     * @param v value to encode
     * @return reference to stream
     */
    template <class... T>
    ScaleEncoderStream &operator<<(const boost::variant<T...> &v) {
      tryEncodeAsOneOfVariant<0>(v);
      return *this;
    }
#endif  // USE_BOOST_VARIANT

    /**
     * @brief scale-encodes sharead_ptr value
     * @tparam T type list
     * @param v value to encode
     * @return reference to stream
     */
    template <typename T>
    ScaleEncoderStream &operator<<(const std::shared_ptr<T> &v) {
      if (v == nullptr) {
        raise(EncodeError::DEREF_NULLPOINTER);
      }
      return *this << *v;
    }

    /**
     * @brief scale-encodes unique_ptr value
     * @tparam T type list
     * @param v value to encode
     * @return reference to stream
     */
    template <typename T>
    ScaleEncoderStream &operator<<(const std::unique_ptr<T> &v) {
      if (v == nullptr) {
        raise(EncodeError::DEREF_NULLPOINTER);
      }
      return *this << *v;
    }

    /**
     * @brief scale-encodes optional value
     * @tparam T value type
     * @param v value to encode
     * @return reference to stream
     */
    template <typename T>
    ScaleEncoderStream &operator<<(const std::optional<T> &v) {
      // optional bool is a special case of optional values
      // it should be encoded using one byte instead of two
      // as described in specification
      if constexpr (std::is_same_v<T, bool>) {
        return encodeOptionalBool(v);
      }
      if (!v.has_value()) {
        return putByte(0u);
      }
      return putByte(1u) << *v;
    }

    /**
     * @brief scale-encodes nullopt  value
     * @param v - std::nullopt only
     * @return reference to stream
     */
    ScaleEncoderStream &operator<<(const std::nullopt_t &) {
      return putByte(0u);
    }

    /**
     * @brief scale-encodes std::reference_wrapper of a type
     * @tparam T underlying type
     * @param v value to encode
     * @return reference to stream;
     */
    template <typename T>
    ScaleEncoderStream &operator<<(const std::reference_wrapper<T> &v) {
      return *this << static_cast<const T &>(v);
    }

    /**
     * @brief scale-encodes a string view
     * @param sv string_view item
     * @return reference to stream
     */
    ScaleEncoderStream &operator<<(std::string_view sv) {
      return encodeDynamicCollection(sv);
    }

    /**
     * @brief scale-encodes a vector of bool
     * @param v vector of bool
     * @return reference to stream
     */
    ScaleEncoderStream &operator<<(const std::vector<bool> &v) {
      *this << CompactInteger{v.size()};
      for (bool el : v) {
        *this << el;
      }
      return *this;
    }

    /**
     * @brief scale-encodes any integral type including bool
     * @tparam T integral type
     * @param v value of integral type
     * @return reference to stream
     */
    template <typename T>
      requires std::is_integral_v<std::decay_t<T>>
    ScaleEncoderStream &operator<<(T &&v) {
      using I = std::decay_t<T>;
      // encode bool
      if constexpr (std::is_same_v<I, bool>) {
        uint8_t byte = (v ? 1u : 0u);
        return putByte(byte);
      }
      // put byte
      if constexpr (sizeof(T) == 1u) {
        // to avoid infinite recursion
        return putByte(static_cast<uint8_t>(v));
      }
      // encode any other integer
      detail::encodeInteger<I>(v, *this);
      return *this;
    }

    /**
     * @brief scale-encodes CompactInteger value as compact integer
     * @param v value to encode
     * @return reference to stream
     */
    ScaleEncoderStream &operator<<(const CompactInteger &v);

   protected:
    template <size_t I, class... Ts>
    void encodeElementOfTuple(const std::tuple<Ts...> &v) {
      *this << std::get<I>(v);
      if constexpr (sizeof...(Ts) > I + 1) {
        encodeElementOfTuple<I + 1>(v);
      }
    }

    template <uint8_t I, class... Ts>
    void tryEncodeAsOneOfVariant(const std::variant<Ts...> &v) {
      using T = std::tuple_element_t<I, std::tuple<Ts...>>;
      if (v.index() == I) {
        *this << I << std::get<T>(v);
        return;
      }
      if constexpr (sizeof...(Ts) > I + 1) {
        tryEncodeAsOneOfVariant<I + 1>(v);
      }
    }

#ifdef USE_BOOST_VARIANT
    template <uint8_t I, class... Ts>
    void tryEncodeAsOneOfVariant(const boost::variant<Ts...> &v) {
      using T = std::tuple_element_t<I, std::tuple<Ts...>>;
      if (v.type() == typeid(T)) {
        *this << I << boost::get<T>(v);
        return;
      }
      if constexpr (sizeof...(Ts) > I + 1) {
        tryEncodeAsOneOfVariant<I + 1>(v);
      }
    }
#endif  // USE_BOOST_VARIANT

    /**
     * @brief scale-encodes any dynamic collection
     * @param collection encoding collection
     * @return reference to stream
     */
    ScaleEncoderStream &encodeDynamicCollection(
        const std::ranges::sized_range auto &collection) {
      *this << CompactInteger{collection.size()};
      for (const auto &item : collection) {
        *this << item;
      }
      return *this;
    }

    /**
     * @brief scale-encodes any static (fixed-size) collection
     * @param collection encoding collection
     * @return reference to stream
     */
    ScaleEncoderStream &encodeStaticCollection(
        const StaticCollection auto &collection) {
      for (const auto &item : collection) {
        *this << item;
      }
      return *this;
    }

    /**
     * @brief puts a byte to buffer
     * @param v byte value
     * @return reference to stream
     */
    ScaleEncoderStream &putByte(uint8_t v);

   private:
    ScaleEncoderStream &encodeOptionalBool(const std::optional<bool> &v);

    const bool drop_data_ = false;
    std::deque<uint8_t> stream_;
    size_t bytes_written_ = 0;
  };

  /**
   * @brief scale-encodes any enum type as its underlying type
   * Defined outside ScaleEncoderStream to allow custom overloads for
   * specific enum types.
   * @tparam T enum type
   * @param v value of the enum type
   * @return reference to stream
   */
  template <typename S, typename T>
    requires std::is_base_of_v<ScaleEncoderStream, S>
             and std::is_enum_v<std::decay_t<T>>
  S &operator<<(S &s, const T &v) {
    using E = std::decay_t<T>;
    return s << static_cast<std::underlying_type_t<E>>(v);
  }

}  // namespace scale
