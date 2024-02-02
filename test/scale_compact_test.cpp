/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "util/scale.hpp"

using scale::CompactInteger;

#define TEST_COMPACT(v, expected) \
  TEST_SCALE_ENCODE_DECODE(CompactInteger{v}, expected##_unhex)

TEST(Compact, Test) {
  CompactInteger bit67 = CompactInteger{1} << (8 * 67);
  // 0 is min compact integer value, negative values are not allowed
  TEST_COMPACT(0, "00");
  // 1 is encoded as 4
  TEST_COMPACT(1, "04");
  // max 1 byte value
  TEST_COMPACT(0x3f, "fc");
  // min 2 bytes value
  TEST_COMPACT(0x40, "0101");
  // some 2 bytes value
  TEST_COMPACT(0xff, "fd03");
  // some 2 bytes value
  TEST_COMPACT(0x1ff, "fd07");
  // max 2 bytes value
  TEST_COMPACT(0x3fff, "fdff");
  // min 4 bytes value
  TEST_COMPACT(0x4000, "02000100");
  // some 4 bytes value
  TEST_COMPACT(0xffff, "feff0300");
  // max 4 bytes value
  TEST_COMPACT(0x3fffffff, "feffffff");
  // min multibyte integer
  TEST_COMPACT(0x40000000, "0300000040");
  // some multibyte integer
  TEST_COMPACT("0x3a0c92075c0dbf3b8acbc5f96ce3f0ad2",
               "37d20a3fce965fbcacb8f3dbc07520c9a003");
  // max multibyte integer
  TEST_COMPACT(
      bit67 - 1,
      "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
  TEST_SCALE_ENCODE_ERROR(CompactInteger{-1},
                          EncodeError::NEGATIVE_COMPACT_INTEGER);
  TEST_SCALE_ENCODE_ERROR(bit67, EncodeError::COMPACT_INTEGER_TOO_BIG);
}
