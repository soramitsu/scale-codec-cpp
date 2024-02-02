/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "util/scale.hpp"

TEST(Array, Test) {
  TEST_SCALE_ENCODE_DECODE(std::array<uint8_t, 1>{0xca}, "ca"_unhex);
  TEST_SCALE_ENCODE_DECODE(std::array<uint8_t, 2>{0xca, 0xfe}, "cafe"_unhex);
}
