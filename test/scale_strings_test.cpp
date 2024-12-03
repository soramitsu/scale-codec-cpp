/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include <scale/scale.hpp>

using scale::ByteArray;
using scale::ScaleDecoderStream;
using scale::ScaleEncoderStream;

/**
 * @given raw string
 * @when specified string is encoded by ScaleEncoderStream
 * @then encoded value meets expectations
 */
TEST(StringTest, RawStringEncodeSuccess) {
  auto *v = "abcdef";
  ScaleEncoderStream s{};
  ASSERT_NO_THROW((s << v));
  auto buff = s.to_vector();

  auto actual = std::span(buff).last(buff.size() - 1);
  auto expected = ByteArray{'a', 'b', 'c', 'd', 'e', 'f'};

  ASSERT_TRUE(std::ranges::equal(actual, expected));
}

/**
 * @given raw string
 * @when specified string is encoded by ScaleEncoderStream
 * @then encoded value meets expectations
 */
TEST(StringTest, StdStringEncodeSuccess) {
  std::string v = "abcdef";
  ScaleEncoderStream s;
  ASSERT_NO_THROW((s << v));
  auto buff = s.to_vector();

  auto actual = std::span(buff).last(buff.size() - 1);
  auto expected = ByteArray{'a', 'b', 'c', 'd', 'e', 'f'};

  ASSERT_TRUE(std::ranges::equal(actual, expected));
}

/**
 * @given byte array containing encoded string
 * @when string is decoded using ScaleDecoderStream
 * @then decoded string matches expectations
 */
TEST(StringTest, StringDecodeSuccess) {
  std::string i = "abcdef";
  ScaleEncoderStream es;
  ASSERT_NO_THROW((es << i));
  auto encoded = es.to_vector();

  ScaleDecoderStream ds(encoded);
  std::string o;
  ds >> o;

  ASSERT_TRUE(std::ranges::equal(i, o));
}
