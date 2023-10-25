/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <scale/buffer/hexutil.hpp>

inline std::vector<uint8_t> operator""_unhex(const char *c, size_t s) {
  if (s > 2 and c[0] == '0' and c[1] == 'x')
    return scale::unhexWith0x(std::string_view(c, s)).value();
  return scale::unhex(std::string_view(c, s)).value();
}
