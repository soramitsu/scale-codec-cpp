/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SCALE_TESTUTIL_LITERALS_HPP
#define SCALE_TESTUTIL_LITERALS_HPP

#include <scale/buffer/buffer.hpp>
#include <scale/buffer/hexutil.hpp>

/// creates a buffer filled with characters from the original string
/// mind that it does not perform unhexing, there is ""_unhex for it
inline scale::Buffer operator"" _buf(const char *c, size_t s) {
  std::vector<uint8_t> chars(c, c + s);
  return scale::Buffer(std::move(chars));
}

inline std::vector<uint8_t> operator"" _v(const char *c, size_t s) {
  std::vector<uint8_t> chars(c, c + s);
  return chars;
}

inline scale::Buffer operator"" _hex2buf(const char *c, size_t s) {
  return scale::Buffer::fromHex(std::string_view(c, s)).value();
}

inline std::vector<uint8_t> operator""_unhex(const char *c, size_t s) {
  if (s > 2 and c[0] == '0' and c[1] == 'x')
    return scale::unhexWith0x(std::string_view(c, s)).value();
  return scale::unhex(std::string_view(c, s)).value();
}

#endif  // SCAKE_TESTUTIL_LITERALS_HPP
