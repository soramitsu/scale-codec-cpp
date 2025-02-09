/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include <scale/scale.hpp>

using scale::ScaleEncoderStream;

class ScaleCounter : public ::testing::Test {
 public:
  ScaleCounter() : s(true) {}

 protected:
  ScaleEncoderStream s;
};

struct TestStruct {
  uint8_t x;
  std::string y;
};

// helper for same kind checks
#define SIZE(bytes) ASSERT_EQ(s.size(), bytes)

/**
 * @given a bool
 * @when it gets scale encoded
 * @then the resulting stream size equals to expected
 */
TEST_F(ScaleCounter, Bool) {
  s << true;
  SIZE(1);
}

/**
 * @given a string
 * @when it gets scale encoded
 * @then the resulting stream size equals to expected
 */
TEST_F(ScaleCounter, String) {
  std::string value = "test string";
  s << value;
  SIZE(value.size() + 1);
}

/**
 * @given an empty optional
 * @when it gets scale encoded
 * @then the resulting stream size equals to expected
 */
TEST_F(ScaleCounter, EmptyOptional) {
  std::optional<uint32_t> var = std::nullopt;
  s << var;
  SIZE(1);
}

/**
 * @given an optional with an uint32 value inside
 * @when it gets scale encoded
 * @then the resulting stream size equals to expected
 */
TEST_F(ScaleCounter, NonEmptyOptional) {
  std::optional<uint32_t> var = 10;
  s << var;
  SIZE(5);
}

/**
 * @given a custom defined struct
 * @when it gets scale encoded
 * @then the resulting stream size equals to expected
 */
TEST_F(ScaleCounter, CustomStruct) {
  TestStruct st{.x = 10, .y = "test string"};
  s << st;
  SIZE(1 + st.y.size() + 1);
}
