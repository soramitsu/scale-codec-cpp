/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "util/scale.hpp"

using T1 = uint8_t;
using T2 = uint16_t;
using T3 = uint32_t;

#define TEST_TUPLE(type) \
  TEST_SCALE_ENCODE_DECODE(type{1, 2, 3}, "01020003000000"_unhex)

struct Tie1 {
  T1 t1;
  T2 t2;
  T3 t3;
};
struct Tie2 {
  T1 t1;
  T2 t2;
  T3 t3;
  int t4 = 4;
  auto tie() {
    return std::tie(t1, t2, t3);
  }
};
struct Pairs {
  Pairs() = default;
  Pairs(T1 t1, T2 t2, T3 t3) : t{{t1, t2}, t3} {}
  std::pair<std::pair<T1, T2>, T3> t;
};

TEST(Tuple, Test) {
  TEST_TUPLE((std::tuple<T1, T2, T3>));
  TEST_TUPLE(Tie1);
  TEST_TUPLE(Tie2);
  TEST_TUPLE(Pairs);
}
