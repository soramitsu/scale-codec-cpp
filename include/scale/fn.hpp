/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <boost/variant.hpp>
#include <memory>
#include <optional>
#include <scale/bitvec.hpp>
#include <scale/fn.fwd.hpp>
#include <scale/outcome/catch.hpp>
#include <scale/outcome/outcome_throw.hpp>
#include <scale/scale_error.hpp>
#include <scale/tie.hpp>
#include <scale/to_le.hpp>
#include <scale/types.hpp>
#include <variant>

namespace scale {
  outcome::result<void> encodeFn(_SCALE_FN, const auto &...v) {
    return outcomeCatch([&] { encodeTo(FnTag{}, fn, v...); });
  }

  void encodeTo(SCALE_FN, const auto &v0, const auto &v1, const auto &...v) {
    encodeTo(tag, fn, v0);
    encodeTo(tag, fn, v1, v...);
  }

  template <std::integral T>
  void encodeTo(SCALE_FN, const T &v) {
    if constexpr (std::is_same_v<T, bool>) {
      encodeTo(tag, fn, static_cast<uint8_t>(v ? 1 : 0));
    } else {
      auto le = toLE(v);
      fn({reinterpret_cast<const uint8_t *>(&le), sizeof(T)});
    }
  }

  template <typename T>
  void encodeTo(SCALE_FN, const T &v)
    requires(std::is_enum_v<T>)
  {
    encodeTo(tag, fn, static_cast<std::underlying_type_t<T>>(v));
  }

  template <typename T>
  size_t bitCount(const T &v) {
    if constexpr (std::is_same_v<T, CompactInteger>) {
      return v.is_zero() ? 0 : msb(v) + 1;
    } else {
      return 64 - __builtin_clzll(uint64_t{v});
    }
  }

  void encodeCompact(SCALE_FN, uint64_t v, size_t bits) {
    auto mask = [&]<typename T>(T v, auto mask) {
      encodeTo(tag, fn, static_cast<T>((v << 2) | mask));
    };
    if (bits <= 8 - 2) {
      return mask(static_cast<uint8_t>(v), 0);
    }
    if (bits <= 16 - 2) {
      return mask(static_cast<uint16_t>(v), 1);
    }
    if (bits <= 32 - 2) {
      return mask(static_cast<uint32_t>(v), 2);
    }
    auto bytes = (bits + 7) / 8;
    uint8_t raw[1 + sizeof(uint64_t)];
    raw[0] = ((bytes - 4) << 2) | 3;
    *reinterpret_cast<uint64_t *>(raw + 1) = toLE(v);
    fn({raw, 1 + bytes});
  }

  void encodeCompact(SCALE_FN, uint64_t v) {
    encodeCompact(tag, fn, v, bitCount(v));
  }

  void encodeTo(SCALE_FN, const std::same_as<CompactInteger> auto &v) {
    if (v.sign() < 0) {
      raise(EncodeError::NEGATIVE_COMPACT_INTEGER);
    }
    auto bits = bitCount(v);
    if (bits <= 64) {
      return encodeCompact(tag, fn, uint64_t{v}, bits);
    }
    if (bits > 8 * (4 + 63)) {
      raise(EncodeError::COMPACT_INTEGER_TOO_BIG);
    }
    auto bytes = (bits + 7) / 8;
    uint8_t raw[1 + 4 + 63];
    raw[0] = ((bytes - 4) << 2) | 3;
    export_bits(v, raw + 1, 8, false);
    fn({raw, 1 + bytes});
  }

  void encodeTo(SCALE_FN, const EncodeCompact &v) {
    encodeCompact(tag, fn, v.v);
  }

  void encodeTo(SCALE_FN, const EncodeRaw &v) {
    fn(v.v);
  }

  template <typename T>
  void encodeTo(SCALE_FN, const std::optional<T> &v) {
    if constexpr (std::is_same_v<T, bool>) {
      encodeTo(tag, fn, static_cast<uint8_t>(v ? *v ? 1 : 2 : 0));
    } else {
      encodeTo(tag, fn, static_cast<uint8_t>(v ? 1 : 0));
      if (v) {
        encodeTo(tag, fn, *v);
      }
    }
  }

  template <typename... T>
  void encodeTo(SCALE_FN, const std::variant<T...> &v) {
    static_assert(sizeof...(T) <= 0xff);
    encodeTo(tag, fn, static_cast<uint8_t>(v.index()));
    visit([&](const auto &v) { encodeTo(tag, fn, v); }, v);
  }

  template <typename... T>
  void encodeTo(SCALE_FN, const boost::variant<T...> &v) {
    static_assert(sizeof...(T) <= 0xff);
    encodeTo(tag, fn, static_cast<uint8_t>(v.which()));
    apply_visitor([&](const auto &v) { encodeTo(tag, fn, v); }, v);
  }

  template <typename... T>
  void encodeTo(SCALE_FN, const std::tuple<T...> &v) {
    if constexpr (sizeof...(T) != 0) {
      std::apply([&](const auto &...v) { encodeTo(tag, fn, v...); }, v);
    }
  }

  void encodeTo(SCALE_FN, const std::vector<bool> &v) {
    encodeCompact(tag, fn, v.size());
    for (bool x : v) {
      encodeTo(tag, fn, x);
    }
  }

  void encodeTo(SCALE_FN, const BitVec &v) {
    encodeCompact(tag, fn, v.bits.size());
    uint64_t mask = 1;
    uint64_t value = 0;
    for (auto bit : v.bits) {
      if (bit) {
        value |= mask;
      }
      if ((mask & 0x8000000000000000) == 0) {
        mask <<= 1;
      } else {
        mask = 1;
        encodeTo(tag, fn, value);
        value = 0;
      }
    }
    if ((mask & 1) == 0) {
      auto bits = bitCount(mask) - 1;
      auto bytes = (bits + 7) / 8;
      auto le = toLE(value);
      fn({reinterpret_cast<const uint8_t *>(&le), bytes});
    }
  }

  template <IsTie T>
    requires(not std::ranges::sized_range<T>)
  void encodeTo(SCALE_FN, const T &v) {
    encodeTo(tag, fn, ::scale::tie(v));
  }

  void encodeTo(SCALE_FN, const std::ranges::sized_range auto &v) {
    if constexpr (requires { std::span{v}; }) {
      std::span s{v};
      using S = decltype(s);
      if constexpr (S::extent == std::dynamic_extent) {
        encodeCompact(tag, fn, s.size());
      }
      using X = std::remove_const_t<typename S::element_type>;
      if constexpr (std::is_same_v<X, uint8_t>) {
        return fn(s);
      }
      if constexpr (std::is_same_v<X, char>) {
        // TODO: return fn(qtils::str2byte(s));
        return fn(
            BytesIn{reinterpret_cast<const uint8_t *>(s.data()), s.size()});
      }
    } else {
      encodeCompact(tag, fn, v.size());
    }
    for (auto &x : v) {
      encodeTo(tag, fn, x);
    }
  }

  void encodeTo(SCALE_FN, const std::nullptr_t &) = delete;

  void encodeTo(SCALE_FN, const auto *) = delete;

  template <typename T>
  void encodeTo(SCALE_FN, const std::shared_ptr<T> &v) {
    if (v == nullptr) {
      raise(EncodeError::DEREF_NULLPOINTER);
    }
    encodeTo(tag, fn, *v);
  }
}  // namespace scale
