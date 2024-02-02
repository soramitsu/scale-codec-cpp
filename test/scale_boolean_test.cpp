/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "util/scale.hpp"

TEST(Bool, Test) {
  TEST_SCALE_ENCODE_DECODE(false, "00"_unhex);
  TEST_SCALE_ENCODE_DECODE(true, "01"_unhex);
  TEST_SCALE_DECODE_ERROR(bool, "02"_unhex, DecodeError::UNEXPECTED_VALUE);
}
