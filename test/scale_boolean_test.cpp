/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include <qtils/test/outcome.hpp>
#include <scale/scale.hpp>

using scale::ByteArray;
using scale::DecodeError;
using scale::EncodeError;
using scale::ScaleDecoderStream;
using scale::ScaleEncoderStream;

/**
 * @given bool values: true and false
 * @when encode them by fixedwidth::encodeBool function
 * @then obtain expected result each time
 */
TEST(ScaleBoolTest, EncodeBoolSuccess) {
  {
    ScaleEncoderStream s;
    ASSERT_NO_THROW((s << true));
    ASSERT_EQ(s.to_vector(), (ByteArray{0x1}));
  }
  {
    ScaleEncoderStream s;
    ASSERT_NO_THROW((s << false));
    ASSERT_EQ(s.to_vector(), (ByteArray{0x0}));
  }
}

/**
 * @brief helper structure for testing scale::decode
 */
struct ThreeBooleans {
  bool b1 = false;
  bool b2 = false;
  bool b3 = false;
};

/**
 * @given byte array containing values {0, 1, 2}
 * @when scale::decode function is applied sequentially
 * @then it returns false, true and kUnexpectedValue error correspondingly,
 * and in the end no more bytes left in stream
 */
TEST(Scale, fixedwidthDecodeBoolFail) {
  auto bytes = ByteArray{0, 1, 2};
  EXPECT_EC(scale::decode<ThreeBooleans>(bytes), DecodeError::UNEXPECTED_VALUE);
}

/**
 * @given byte array containing values {0, 1, 2}
 * @when scale::decode function is applied sequentially
 * @then it returns false, true and kUnexpectedValue error correspondingly,
 * and in the end no more bytes left in stream
 */
TEST(Scale, fixedwidthDecodeBoolSuccess) {
  auto bytes = ByteArray{0, 1, 0};
  auto res = EXPECT_OK(scale::decode<ThreeBooleans>(bytes));
  ASSERT_EQ(res.b1, false);
  ASSERT_EQ(res.b2, true);
  ASSERT_EQ(res.b3, false);
}
