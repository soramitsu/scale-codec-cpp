/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <deque>
#include <map>

#include "util/scale.hpp"

using scale::BitVec;
using scale::CompactInteger;

TEST(Scale, encodeBytes) {
  TEST_SCALE_ENCODE_DECODE("61736461646164"_unhex, "1c61736461646164"_unhex);
}

/**
 * @given vector of bools
 * @when encodeCollection is applied
 * @then expected result is obtained: header is 2 byte, items are 1 byte each
 */
TEST(Scale, encodeVectorOfBool) {
  std::vector<bool> collection = {true, false, true, false, false, false};
  TEST_SCALE_ENCODE_DECODE(collection, "18010001000000"_unhex);
}

TEST(Scale, encodeBitVec) {
  auto v = BitVec{{true, true, false, false, false, false, true}};
  TEST_SCALE_ENCODE_DECODE(v, "1c43"_unhex);

  BitVec v2;
  v2.bits.resize(9 * 8);
  for (size_t i = 0; i < 9; ++i) {
    v2.bits[8 * i + i % 8] = true;
  }
  v2.bits.back() = true;
  TEST_SCALE_ENCODE_DECODE(v2, "2101010204081020408081"_unhex);
}

/**
 * @given collection of items of type uint16_t
 * @when encodeCollection is applied
 * @then expected result is obtained
 */
TEST(Scale, Explicit) {
  std::vector<uint16_t> vector = {1, 2, 3, 4};
  std::array<uint16_t, 4> array = {1, 2, 3, 4};
  std::deque<uint16_t> deque = {1, 2, 3, 4};
  TEST_SCALE_ENCODE_DECODE(vector, "100100020003000400"_unhex);
  TEST_SCALE_ENCODE_DECODE(array, "0100020003000400"_unhex);
  TEST_SCALE_ENCODE_DECODE(deque, "100100020003000400"_unhex);
}

/**
 * @given map of <uint32_t, uint32_t>
 * @when encodeCollection is applied
 * @then expected result is obtained: header is 2 byte, items are pairs of 4
 * byte elements each
 */
TEST(Scale, encodeMapTest) {
  std::map<uint32_t, uint32_t> collection = {{1, 5}, {2, 6}, {3, 7}, {4, 8}};
  TEST_SCALE_ENCODE_DECODE(
      collection,
      "100100000005000000020000000600000003000000070000000400000008000000"_unhex);
}

/**
 * @given encoded 3-elements collection
 * @when decode it to collection limited by size 4, 3 and 2 max size
 * @then if max_size is enough, it is done successful, and error otherwise
 */
TEST(Scale, decodeSizeLimitedCollection) {
  struct MaxSize : std::vector<uint16_t> {
    size_type max_size() const {
      return 3;
    }
  };
  TEST_SCALE_ENCODE_DECODE((MaxSize{{1, 2, 3}}), "0c010002000300"_unhex);
  TEST_SCALE_DECODE_ERROR(
      MaxSize, "100100020003000400"_unhex, DecodeError::TOO_MANY_ITEMS);
}
