/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "util/scale.hpp"

TEST(Optional, Test) {
  using T1 = std::optional<uint8_t>;
  TEST_SCALE_ENCODE_DECODE(T1{}, "00"_unhex);
  TEST_SCALE_ENCODE_DECODE(T1{2}, "0102"_unhex);
  TEST_SCALE_DECODE_ERROR(T1, "02"_unhex, DecodeError::UNEXPECTED_VALUE);

  using T2 = std::optional<bool>;
  TEST_SCALE_ENCODE_DECODE(T2{}, "00"_unhex);
  TEST_SCALE_ENCODE_DECODE(T2{true}, "01"_unhex);
  TEST_SCALE_ENCODE_DECODE(T2{false}, "02"_unhex);
  TEST_SCALE_DECODE_ERROR(T1, "03"_unhex, DecodeError::UNEXPECTED_VALUE);
}
