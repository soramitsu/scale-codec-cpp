/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "util/scale.hpp"

TEST(String, Test) {
  TEST_SCALE_ENCODE_DECODE(std::string{"asdadad"}, "1c61736461646164"_unhex);
}
