/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <gtest/gtest.h>
#include <scale/encode.hpp>
#include <scale/scale.hpp>
#include <type_traits>

#include "outcome.hpp"

// TODO(turuslan): using qtils::operator""_unhex;
#include <boost/algorithm/hex.hpp>
inline auto operator""_unhex(const char *c, size_t s) {
  assert(s % 2 == 0);
  std::vector<uint8_t> v(s / 2);
  boost::algorithm::unhex(c, c + s, v.begin());
  return v;
}

using scale::DecodeError;
using scale::EncodeError;

#define TEST_SCALE_ENCODE_DECODE                            \
  [](const auto &v, const std::vector<uint8_t> &expected) { \
    using T = std::remove_cvref_t<decltype(v)>;             \
    auto actual = scale::encode(v).value();                 \
    EXPECT_EQ(actual, expected);                            \
    auto v2 = scale::decode<T>(expected).value();           \
    auto actual2 = scale::encode(v2).value();               \
    EXPECT_EQ(actual2, expected) << "reencode";             \
  }
#define TEST_SCALE_ENCODE_ERROR(v, expected) \
  EXPECT_EC(scale::encode(v), expected)
#define TEST_SCALE_DECODE_ERROR(type, raw, expected) \
  EXPECT_EC(scale::decode<type>(raw), expected)
