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

template <typename T>
class EnumTest : public ::testing::Test {
 protected:
  const static std::string enum_name;
  const static std::vector<T> values;
};

enum class Foo : uint16_t { A = 0, B = 1, C = 2 };

enum class Bar : int64_t { A = -32, B = 42, C = 0 };

using MyTypes = ::testing::Types<Foo, Bar>;
TYPED_TEST_SUITE(EnumTest, MyTypes);

template <>
const std::string EnumTest<Foo>::enum_name{"Foo"};

SCALE_DEFINE_ENUM_VALUE_RANGE(, Foo, Foo::A, Foo::C);

template <>
const std::vector<Foo> EnumTest<Foo>::values{Foo::A, Foo::B, Foo::C};

template <>
const std::string EnumTest<Bar>::enum_name{"Bar"};
template <>
const std::vector<Bar> EnumTest<Bar>::values{Bar::A, Bar::B, Bar::C};

SCALE_DEFINE_ENUM_VALUE_LIST(, Bar, Bar::A, Bar::B, Bar::C);

TYPED_TEST(EnumTest, ConsistentEncodingDecoding) {
  SCOPED_TRACE(TestFixture::enum_name);
  for (auto const &param : TestFixture::values) {
    ScaleEncoderStream encoder{};
    ASSERT_NO_THROW((encoder << param));

    auto v = encoder.to_vector();
    ScaleDecoderStream decoder{v};
    TypeParam decoded_value;
    ASSERT_NO_THROW((decoder >> decoded_value));

    EXPECT_EQ(decoded_value, param);
  }
}

TYPED_TEST(EnumTest, CorrectEncoding) {
  for (auto const &param : TestFixture::values) {
    ScaleEncoderStream encoder{};
    ASSERT_NO_THROW((encoder << param));
    auto v = encoder.to_vector();
    ScaleDecoderStream decoder{v};
    std::underlying_type_t<TypeParam> decoded_value;
    ASSERT_NO_THROW((decoder >> decoded_value));
    EXPECT_EQ(decoded_value,
              static_cast<std::underlying_type_t<TypeParam>>(param));
  }
}

template <typename T>
class InvalidEnumTest : public ::testing::Test {
 protected:
  const static std::string enum_name;
  const static std::vector<std::underlying_type_t<T>> invalid_values;
};

template <>
const std::string InvalidEnumTest<Foo>::enum_name{"Foo"};
template <>
const std::vector<uint16_t> InvalidEnumTest<Foo>::invalid_values{11, 22, 33};

template <>
const std::string InvalidEnumTest<Bar>::enum_name{"Bar"};
template <>
const std::vector<int64_t> InvalidEnumTest<Bar>::invalid_values{1, 2, 3};

using MyTypes = ::testing::Types<Foo, Bar>;
TYPED_TEST_SUITE(InvalidEnumTest, MyTypes);

TYPED_TEST(InvalidEnumTest, ThrowsOnInvalidValue) {
  for (auto const &param : TestFixture::invalid_values) {
    ScaleEncoderStream encoder{};
    ASSERT_NO_THROW((encoder << param));
    auto v = encoder.to_vector();
    ScaleDecoderStream decoder{v};
    TypeParam decoded_value;
    ASSERT_THROW((decoder >> decoded_value), std::runtime_error);
  }
}
