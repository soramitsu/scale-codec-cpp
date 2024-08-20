/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include <scale/scale.hpp>

using scale::ByteArray;
using scale::ScaleDecoderStream;
using scale::ScaleEncoderStream;

/**
 * @given pair of values of different types: uint8_t and uint32_t
 * @when encode is applied
 * @then obtained serialized value meets predefined one
 */
TEST(Scale, encodePair) {
  uint8_t v1 = 1;
  uint32_t v2 = 2;

  ScaleEncoderStream s;
  ASSERT_NO_THROW((s << std::make_pair(v1, v2)));
  ASSERT_EQ(s.to_vector(), (ByteArray{1, 2, 0, 0, 0}));
}

/**
 * @given byte sequence containign 2 encoded values of
 * different types: uint8_t and uint32_t
 * @when decode is applied
 * @then obtained pair mathces predefined one
 */
TEST(Scale, decodePair) {
  ByteArray bytes = {1, 2, 0, 0, 0};
  ScaleDecoderStream s(bytes);
  using pair_type = std::pair<uint8_t, uint32_t>;
  pair_type pair{};
  ASSERT_NO_THROW((s >> pair));
  ASSERT_EQ(pair.first, 1);
  ASSERT_EQ(pair.second, 2);
}
