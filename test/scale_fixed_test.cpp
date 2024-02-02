/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "util/scale.hpp"

#define TEST_FIXED(v, expected) \
  TEST_SCALE_ENCODE_DECODE(Type{v}, expected##_unhex)

TEST(Fixed, Int8) {
  using Type = int8_t;
  TEST_FIXED(0, "00");
  TEST_FIXED(-1, "ff");
  TEST_FIXED(-128, "80");
  TEST_FIXED(-127, "81");
  TEST_FIXED(123, "7b");
  TEST_FIXED(-15, "f1");
}

TEST(Fixed, Uint8) {
  using Type = uint8_t;
  TEST_FIXED(0, "00");
  TEST_FIXED(234, "ea");
  TEST_FIXED(255, "ff");
}

TEST(Fixed, Int16) {
  using Type = int16_t;
  TEST_FIXED(-32767, "0180");
  TEST_FIXED(-32768, "0080");
  TEST_FIXED(-1, "ffff");
  TEST_FIXED(32767, "ff7f");
  TEST_FIXED(12345, "3930");
  TEST_FIXED(-12345, "c7cf");
}

TEST(Fixed, Uint16) {
  using Type = uint16_t;
  TEST_FIXED(32767, "ff7f");
  TEST_FIXED(12345, "3930");
}

TEST(Fixed, Int32) {
  using Type = int32_t;
  TEST_FIXED(2147483647, "ffffff7f");
  TEST_FIXED(-1, "ffffffff");
  TEST_FIXED(1, "01000000");
}

TEST(Fixed, Uint32) {
  using Type = uint32_t;
  TEST_FIXED(16909060, "04030201");
  TEST_FIXED(67305985, "01020304");
}

TEST(Fixed, Int64) {
  using Type = int64_t;
  TEST_FIXED(578437695752307201, "0102030405060708");
  TEST_FIXED(-1, "ffffffffffffffff");
}

TEST(Fixed, Uint64) {
  using Type = uint64_t;
  TEST_FIXED(578437695752307201, "0102030405060708");
}
