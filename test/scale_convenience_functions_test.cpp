/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include <qtils/test/outcome.hpp>
#include <scale/scale.hpp>

using scale::decode;
using scale::encode;

struct TestStruct {
  std::string a;
  int b;

  inline bool operator==(const TestStruct &rhs) const {
    return a == rhs.a && b == rhs.b;
  }
};

template <class Stream, typename = std::enable_if_t<Stream::is_encoder_stream>>
Stream &operator<<(Stream &s, const TestStruct &test_struct) {
  return s << test_struct.a << test_struct.b;
}

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

  auto encoded = EXPECT_OK(encode(s1));
  auto decoded = EXPECT_OK(decode<TestStruct>(encoded));

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

  auto encoded = EXPECT_OK(encode(expected_string, expected_int));
  auto decoded = EXPECT_OK(decode<TestStruct>(encoded));

  ASSERT_EQ(decoded.a, expected_string);
  ASSERT_EQ(decoded.b, expected_int);
}
