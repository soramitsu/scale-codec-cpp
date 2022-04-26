/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SCALE_CORE_SCALE_SCALE_ENCODER_STREAM_HPP
#define SCALE_CORE_SCALE_SCALE_ENCODER_STREAM_HPP

#include <deque>
#include <optional>

#include <boost/variant.hpp>
#include <gsl/span>

#include <scale/detail/fixed_width_integer.hpp>

namespace scale {

  /**
   * @class ScaleEncoderStream designed to scale-encode data to stream
   */
  class ScaleEncoderStream {
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

    /**
     * @return vector of bytes containing encoded data
     */
    std::vector<uint8_t> to_vector() const;

    /**
     * Get amount of encoded data written to the stream
     * @return size in bytes
     */
    size_t size() const;

    /**
     * @brief scale-encodes std::vector
     * @tparam T type of item
     * @param c collection to encode
     * @return reference to stream
     */
    template <typename T>
    ScaleEncoderStream &operator<<(const std::vector<T> &c) {
      return encodeDynamicCollection(std::size(c), std::begin(c), std::end(c));
    }
    /**
     * @brief scale-encodes std::deque
     * @tparam T type of item
     * @param c collection to encode
     * @return reference to stream
     */
    template <typename T>
    ScaleEncoderStream &operator<<(const std::deque<T> &c) {
      return encodeDynamicCollection(std::size(c), std::begin(c), std::end(c));
    }
    /**
     * @brief scale-encodes std::list
     * @tparam T type of item
     * @param c collection to encode
     * @return reference to stream
     */
    template <typename T>
    ScaleEncoderStream &operator<<(const std::list<T> &c) {
      return encodeDynamicCollection(std::size(c), std::begin(c), std::end(c));
    }
    /**
     * @brief scale-encodes std::map
     * @tparam K key type
     * @tparam V mapped type
     * @param c collection to encode
     * @return reference to stream
     */
    template <typename K, typename V>
    ScaleEncoderStream &operator<<(const std::map<K, V> &c) {
      return encodeDynamicCollection(std::size(c), std::begin(c), std::end(c));
    }

    /**
     * @brief scale-encodes gsl::span
     * @tparam T type of item
     * @tparam S container size (-1 for dynamic)
     * @param span span to encode
     * @return reference to stream
     */
    template <typename T, ssize_t S>
    ScaleEncoderStream &operator<<(const gsl::span<T, S> &span) {
      if constexpr (S == -1) {
        return encodeDynamicCollection(
            std::size(span), std::begin(span), std::end(span));
      } else {
        return encodeStaticCollection(span);
      }
    }

    /**
     * @brief scale-encodes collection of same type items, requires bool tag
     * is_static_collection
     * @tparam C container type, T type of item
     * @param c collection to encode
     * @return reference to stream
     */
    template <class C,
              typename T = typename C::value_type,
              typename = std::enable_if_t<C::is_static_collection
                                          || !C::is_static_collection>>
    ScaleEncoderStream &operator<<(const C &c) {
      if constexpr (C::is_static_collection) {
        return encodeStaticCollection(c);
      } else {
        return encodeDynamicCollection(
            std::size(c), std::begin(c), std::end(c));
      }
    }

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
    ScaleEncoderStream &operator<<(const boost::variant<T...> &v) {
      tryEncodeAsOneOfVariant<0>(v);
      return *this;
    }

    /**
     * @brief scale-encodes sharead_ptr value
     * @tparam T type list
     * @param v value to encode
     * @return reference to stream
     */
    template <class T>
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
    template <class T>
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
    template <class T>
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
     * @brief scale-encodes array of items
     * @tparam T item type
     * @tparam size of the array
     * @param a reference to the array
     * @return reference to stream
     */
    template <typename T, size_t size>
    ScaleEncoderStream &operator<<(const std::array<T, size> &a) {
      return encodeStaticCollection(a);
    }

    /**
     * @brief scale-encodes std::reference_wrapper of a type
     * @tparam T underlying type
     * @param v value to encode
     * @return reference to stream;
     */
    template <class T>
    ScaleEncoderStream &operator<<(const std::reference_wrapper<T> &v) {
      return *this << static_cast<const T &>(v);
    }

    /**
     * @brief scale-encodes a string view
     * @param sv string_view item
     * @return reference to stream
     */
    ScaleEncoderStream &operator<<(std::string_view sv) {
      return encodeDynamicCollection(sv.size(), sv.begin(), sv.end());
    }

    /**
     * @brief scale-encodes any integral type including bool
     * @tparam T integral type
     * @param v value of integral type
     * @return reference to stream
     */
    template <typename T,
              typename I = std::decay_t<T>,
              typename = std::enable_if_t<std::is_integral_v<I>>>
    ScaleEncoderStream &operator<<(T &&v) {
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

    /**
     * @brief scale-encodes any dynamic collection
     * @tparam It iterator over collection of bytes
     * @param size size of the collection
     * @param begin iterator pointing to the begin of collection
     * @param end iterator pointing to the end of collection
     * @return reference to stream
     */
    template <class It>
    ScaleEncoderStream &encodeDynamicCollection(const CompactInteger &size,
                                                It &&begin,
                                                It &&end) {
      *this << size;
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      for (auto &&it = begin; it != end; ++it) {
        *this << *it;
      }
      return *this;
    }

    /**
     * @brief scale-encodes any fixed-size collection (std::array)
     * @tparam C collection type
     * @param c collection
     * @return reference to stream
     */
    template <class C>
    ScaleEncoderStream &encodeStaticCollection(const C &c) {
      for (const auto &e : c) {
        *this << e;
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

    const bool drop_data_;
    std::deque<uint8_t> stream_;
    size_t bytes_written_;
  };

  /**
   * @brief scale-encodes any enum type as its underlying type
   * Defined outside ScaleEncoderStream to allow custom overloads for
   * specific enum types.
   * @tparam T enum type
   * @param v value of the enum type
   * @return reference to stream
   */
  template <typename S,
            typename T,
            typename E = std::decay_t<T>,
            typename = std::enable_if_t<S::is_encoder_stream>,
            typename = std::enable_if_t<std::is_enum_v<E>>>
  S &operator<<(S &s, const T &v) {
    return s << static_cast<std::underlying_type_t<E>>(v);
  }

}  // namespace scale

#endif  // SCALE_CORE_SCALE_SCALE_ENCODER_STREAM_HPP
