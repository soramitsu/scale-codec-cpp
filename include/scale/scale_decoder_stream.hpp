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
#include <vector>

#include <boost/variant.hpp>

#include <scale/bitvec.hpp>
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
  class ScaleDecoderStream : public Configurable {
   public:
    // special tag to differentiate decoding streams from others
    static constexpr auto is_decoder_stream = true;

    explicit ScaleDecoderStream(ConstSpanOfBytes data) : span_{data} {}

#ifdef CUSTOM_CONFIG_ENABLED
    explicit ScaleDecoderStream(ConstSpanOfBytes data,
                                const MaybeCofing auto &...config)
        : Configurable(config...), span_{data} {}
#else
    [[deprecated("Scale has compiled without custom config support")]]  //
    ScaleDecoderStream(ConstSpanOfBytes data,
                       const MaybeCofing auto &...config) = delete;
#endif

    template <typename T>
    T decodeCompact() {
      scale::CompactInteger big =
#ifdef JAM_COMPATIBILITY_ENABLED
          detail::decodeJamCompactInteger<CompactInteger>(*this);
#else
          detail::decodeCompactInteger<CompactInteger>(*this);
#endif
      if (not big.is_zero() and msb(big) >= std::numeric_limits<T>::digits) {
        raise(DecodeError::TOO_MANY_ITEMS);
      }
      return static_cast<T>(big);
    }

    size_t decodeLength();

    /**
     * @brief scale-decodes pair of values
     * @tparam F first value type
     * @tparam S second value type
     * @param p pair of values to decode
     * @return reference to stream
     */
    template <class F, class S>
    ScaleDecoderStream &operator>>(std::pair<F, S> &p) {
      static_assert(!std::is_reference_v<F> && !std::is_reference_v<S>);
      return *this >> const_cast<std::remove_const_t<F> &>(p.first)  // NOLINT
             >> const_cast<std::remove_const_t<S> &>(p.second);      // NOLINT
    }

    /**
     * @brief scale-decoding of tuple
     * @tparam T enumeration of tuples types
     * @param v reference to tuple
     * @return reference to stream
     */
    template <class... T>
    ScaleDecoderStream &operator>>(std::tuple<T...> &v) {
      if constexpr (sizeof...(T) > 0) {
        decodeElementOfTuple<0>(v);
      }
      return *this;
    }

    /**
     * @brief scale-decoding of variant
     * @tparam T enumeration of various types
     * @param v reference to variant
     * @return reference to stream
     */
    template <class... Ts>
    ScaleDecoderStream &operator>>(boost::variant<Ts...> &v) {
      // first byte means type index
      uint8_t type_index = 0u;
      *this >> type_index;  // decode type index

      // ensure that index is in [0, types_count)
      if (type_index >= sizeof...(Ts)) {
        raise(DecodeError::WRONG_TYPE_INDEX);
      }

      tryDecodeAsOneOfVariant<0>(v, type_index);
      return *this;
    }

    /**
     * @brief scale-decodes shared_ptr value
     * @tparam T value type
     * @param v value to decode
     * @return reference to stream
     */
    template <class T>
    ScaleDecoderStream &operator>>(std::shared_ptr<T> &v) {
      using mutableT = std::remove_const_t<T>;

      static_assert(std::is_default_constructible_v<mutableT>);

      v = std::make_shared<mutableT>();
      return *this >> const_cast<mutableT &>(*v);  // NOLINT
    }

    /**
     * @brief scale-decodes unique_ptr value
     * @tparam T value type
     * @param v value to decode
     * @return reference to stream
     */
    template <class T>
    ScaleDecoderStream &operator>>(std::unique_ptr<T> &v) {
      using mutableT = std::remove_const_t<T>;

      static_assert(std::is_default_constructible_v<mutableT>);

      v = std::make_unique<mutableT>();
      return *this >> const_cast<mutableT &>(*v);  // NOLINT
    }

    /**
     * @brief scale-decodes any integral type including bool
     * @tparam T integral type
     * @param v value of integral type
     * @return reference to stream
     */
    template <typename T,
              typename I = std::decay_t<T>,
              typename = std::enable_if_t<std::is_integral_v<I>>>
    ScaleDecoderStream &operator>>(T &v) {
      // check bool
      if constexpr (std::is_same_v<I, bool>) {
        v = decodeBool();
        return *this;
      }
      // check byte
      if constexpr (sizeof(T) == 1u) {
        v = nextByte();
        return *this;
      }
      // decode any other integer
      if (not hasMore(sizeof(T))) {
        raise(DecodeError::NOT_ENOUGH_DATA);
      }
      v = boost::endian::
          endian_load<T, sizeof(T), boost::endian::order::little>(
              &span_[current_index_]);
      current_index_ += sizeof(T);
      return *this;
    }

    /**
     * @brief scale-decodes any optional value
     * @tparam T type of optional value
     * @param v optional value reference
     * @return reference to stream
     */
    template <class T>
    ScaleDecoderStream &operator>>(std::optional<T> &v) {
      using mutableT = std::remove_const_t<T>;

      static_assert(std::is_default_constructible_v<mutableT>);

      // optional bool is special case of optional values
      // it is encoded as one byte instead of two
      // as described in specification
      if constexpr (std::is_same_v<mutableT, bool>) {
        v = decodeOptionalBool();
        return *this;
      }
      // detect if optional has value
      bool has_value = false;
      *this >> has_value;
      if (!has_value) {
        v.reset();
        return *this;
      }
      // decode value
      v.emplace();
      return *this >> const_cast<mutableT &>(*v);  // NOLINT
    }

    /**
     * @brief scale-decodes compact integer value
     * @param v compact integer reference
     * @return
     */
    ScaleDecoderStream &operator>>(CompactInteger &v);

    /**
     * @brief scale-decodes to any static (fixed-size) collection
     * @param collection decoding collection to
     * @return reference to stream
     */
    ScaleDecoderStream &operator>>(StaticCollection auto &container) {
      for (auto &item : container) {
        *this >> item;
      }
      return *this;
    }

    /**
     * @brief scale-decodes to resizeable collection (which can be resized first
     * and rewrite elements while decoding)
     * @param collection decoding collection to
     * @return reference to stream
     */
    ScaleDecoderStream &operator>>(ResizeableCollection auto &collection) {
      auto item_count = decodeLength();
      if (item_count > collection.max_size()) {
        raise(DecodeError::TOO_MANY_ITEMS);
      }

      try {
        collection.resize(item_count);
      } catch (const std::bad_alloc &) {
        raise(DecodeError::TOO_MANY_ITEMS);
      }

      for (auto &item : collection) {
        *this >> item;
      }
      return *this;
    }

    /**
     * @brief Specification for vector<bool>
     * @param v reference to container
     * @return reference to stream
     */
    ScaleDecoderStream &operator>>(std::vector<bool> &collection) {
      auto item_count = decodeLength();
      if (item_count > collection.max_size()) {
        raise(DecodeError::TOO_MANY_ITEMS);
      }

      try {
        collection.resize(item_count);
      } catch (const std::bad_alloc &) {
        raise(DecodeError::TOO_MANY_ITEMS);
      }

      bool item;
      for (size_t i = 0u; i < item_count; ++i) {
        *this >> item;
        collection[i] = item;
      }

      return *this;
    }

    /**
     * @brief scale-decodes BitVec
     */
    ScaleDecoderStream &operator>>(BitVec &v);

    /// @note Implementation prohibited as potentially dangerous.
    /// Use manual decoding instead
    ScaleDecoderStream &operator>>(DynamicSpan auto &collection) = delete;

    /**
     * @brief scale-decodes to sequential collection (which can be reserved
     * space first and push element by element back while decoding)
     * @return reference to stream
     */
    ScaleDecoderStream &operator>>(ExtensibleBackCollection auto &collection) {
      using size_type = typename std::decay_t<decltype(collection)>::size_type;

      auto item_count = decodeLength();
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
        *this >> collection.back();
      }
      return *this;
    }

    /**
     * @brief scale-decodes to non-sequential collection (which can not be
     * reserved space or resize, but each element can be emplaced while
     * decoding)
     * @return reference to stream
     */
    ScaleDecoderStream &operator>>(
        RandomExtensibleCollection auto &collection) {
      using size_type = typename std::decay_t<decltype(collection)>::size_type;
      using value_type =
          typename std::decay_t<decltype(collection)>::value_type;

      auto item_count = decodeLength();
      if (item_count > collection.max_size()) {
        raise(DecodeError::TOO_MANY_ITEMS);
      }

      value_type item;

      collection.clear();
      for (size_type i = 0u; i < item_count; ++i) {
        *this >> item;
        try {
          collection.emplace(std::move(item));
        } catch (const std::bad_alloc &) {
          raise(DecodeError::TOO_MANY_ITEMS);
        }
      }
      return *this;
    }

    /**
     * @brief hasMore Checks whether n more bytes are available
     * @param n Number of bytes to check
     * @return True if n more bytes are available and false otherwise
     */
    bool hasMore(uint64_t n) const;

    /**
     * @brief takes one byte from stream and
     * advances current byte iterator by one
     * @return current byte
     */
    uint8_t nextByte();

    using ByteSpan = ConstSpanOfBytes;
    using SpanIterator = ByteSpan::iterator;
    using SizeType = ByteSpan::size_type;

    ByteSpan span() const {
      return span_;
    }
    SizeType currentIndex() const {
      return current_index_;
    }

   private:
    bool decodeBool();
    /**
     * @brief special case of optional values as described in specification
     * @return std::optional<bool> value
     */
    std::optional<bool> decodeOptionalBool();

    template <size_t I, class... Ts>
    void decodeElementOfTuple(std::tuple<Ts...> &v) {
      using T = std::remove_const_t<std::tuple_element_t<I, std::tuple<Ts...>>>;
      *this >> const_cast<T &>(std::get<I>(v));  // NOLINT
      if constexpr (sizeof...(Ts) > I + 1) {
        decodeElementOfTuple<I + 1>(v);
      }
    }

    template <size_t I, class... Ts>
    void tryDecodeAsOneOfVariant(boost::variant<Ts...> &v, size_t i) {
      using T = std::remove_const_t<std::tuple_element_t<I, std::tuple<Ts...>>>;
      static_assert(std::is_default_constructible_v<T>);
      if (I == i) {
        T val;
        *this >> val;
        v = std::forward<T>(val);
        return;
      }
      if constexpr (sizeof...(Ts) > I + 1) {
        tryDecodeAsOneOfVariant<I + 1>(v, i);
      }
    }

    ByteSpan span_;

    SizeType current_index_{0};
  };

}  // namespace scale