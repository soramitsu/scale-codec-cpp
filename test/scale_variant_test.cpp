/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "util/scale.hpp"

TEST(Variant, Test) {
  using T = boost::variant<uint8_t, uint16_t>;
  TEST_SCALE_ENCODE_DECODE(T{uint8_t{1}}, "0001"_unhex);
  TEST_SCALE_ENCODE_DECODE(T{uint16_t{2}}, "010200"_unhex);
  TEST_SCALE_DECODE_ERROR(T, "03"_unhex, DecodeError::WRONG_TYPE_INDEX);
}
