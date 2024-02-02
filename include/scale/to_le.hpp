/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <concepts>
#include <type_traits>

namespace scale {
  template <std::integral T>
  constexpr T toLE(const T &v) {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    if constexpr (sizeof(T) == 8) {
      return static_cast<T>(__builtin_bswap64(v));
    } else if constexpr (sizeof(T) == 4) {
      return static_cast<T>(__builtin_bswap32(v));
    } else if constexpr (sizeof(T) == 2) {
      return static_cast<T>(__builtin_bswap16(v));
    } else {
      static_assert(sizeof(T) == 1);
    }
#endif
    return v;
  }
}  // namespace scale
