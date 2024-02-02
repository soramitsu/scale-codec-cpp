/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include <scale/encode.hpp>
#include <scale/scale.hpp>

#include "util/outcome.hpp"

using scale::decode;
using scale::encode;

struct TestStruct {
  std::string a;
  int b;

  bool operator==(const TestStruct &rhs) const = default;
};

template <class Stream, typename = std::enable_if_t<Stream::is_decoder_stream>>
Stream &operator>>(Stream &s, TestStruct &test_struct) {
  return s >> test_struct.a >> test_struct.b;
}

/**
 * @given encoded TestStruct
 * @when it is decoded back
 * @then we get original TestStruct
 */
TEST(ScaleConvenienceFuncsTest, EncodeSingleValidArgTest) {
  TestStruct s1{"some_string", 42};

  EXPECT_OUTCOME_TRUE(encoded, encode(s1));
  EXPECT_OUTCOME_TRUE(decoded, decode<TestStruct>(encoded));

  ASSERT_EQ(decoded, s1);
}

/**
 * @given encoded string and integer using scale::encode with variadic template
 * @when we decode encoded vector back and put those fields to TestStruct
 * @then we get original string and int
 */
TEST(ScaleConvenienceFuncsTest, EncodeSeveralValidArgTest) {
  std::string expected_string = "some_string";
  int expected_int = 42;

  EXPECT_OUTCOME_TRUE(encoded, encode(expected_string, expected_int));
  EXPECT_OUTCOME_TRUE(decoded, decode<TestStruct>(encoded));

  ASSERT_EQ(decoded.a, expected_string);
  ASSERT_EQ(decoded.b, expected_int);
}
