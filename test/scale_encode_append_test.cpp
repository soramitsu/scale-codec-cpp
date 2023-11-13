/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "scale/encode_append.hpp"

#include <gtest/gtest.h>

#include <scale/scale.hpp>

namespace scale {
  using Values = std::vector<int>;

  TEST(EncodeAppend, Empty) {
    auto value = 1;
    ByteArray out;
    EXPECT_FALSE(
        append_or_new_vec(out, scale::encode(value).value()).has_error());
    EXPECT_EQ(out, scale::encode(Values{value}).value());
  }

  TEST(EncodeAppend, Append) {
    Values values{0, 1, 2, 3, 4};
    auto value = 5;
    auto out = scale::encode(values).value();
    values.emplace_back(value);
    EXPECT_FALSE(
        append_or_new_vec(out, scale::encode(value).value()).has_error());
    EXPECT_EQ(out, scale::encode(values).value());
  }
}  // namespace scale
