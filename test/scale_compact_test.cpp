/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include <qtils/test/outcome.hpp>
#include <scale/scale.hpp>

using scale::ByteArray;
using scale::CompactInteger;
using scale::decode;
using scale::ScaleDecoderStream;
using scale::ScaleEncoderStream;

/**
 * value parameterized tests
 */
class CompactTest
    : public ::testing::TestWithParam<std::pair<CompactInteger, ByteArray>> {
 public:
  static std::pair<CompactInteger, ByteArray> pair(CompactInteger v,
                                                   ByteArray m) {
    return std::make_pair(CompactInteger(std::move(v)), std::move(m));
  }

 protected:
  ScaleEncoderStream s;
};

/**
 * @given a value and corresponding buffer match of its encoding
 * @when value is encoded by means of ScaleEncoderStream
 * @then encoded value matches predefined buffer
 */
TEST_P(CompactTest, EncodeSuccess) {
  const auto &[value, match] = GetParam();
  ASSERT_NO_THROW(s << value) << "Exception while encoding";
  ASSERT_EQ(s.to_vector(), match) << "Encoding fail";
}

/**
 * @given a value and corresponding bytesof its encoding
 * @when value is decoded by means of ScaleDecoderStream from given bytes
 * @then decoded value matches predefined value
 */
TEST_P(CompactTest, DecodeSuccess) {
  const auto &[value_match, bytes] = GetParam();
  ScaleDecoderStream s(bytes);
  CompactInteger v{};
  ASSERT_NO_THROW(s >> v) << "Exception while decoding";
  ASSERT_EQ(v, value_match) << "Decoding fail";
}

#ifdef JAM_COMPATIBILITY_ENABLED

#define BIGGEST_INT_FOR_COMPACT_REPRESENTATION \
  ((CompactInteger(1) << (sizeof(size_t) * CHAR_WIDTH)) - 1)
INSTANTIATE_TEST_SUITE_P(
    CompactTestCases,
    CompactTest,
    ::testing::Values(
        // clang-format off

        // Lowest values for each number of bytes
  /*  0 */ CompactTest::pair(                                                               0b00000000, {0b00000000}),
  /*  1 */ CompactTest::pair(                                                               0b10000000, {0b10000000, 0b10000000}),
  /*  2 */ CompactTest::pair(                                                      0b01000000'00000000, {0b11000000, 0b00000000, 0b01000000}),
  /*  3 */ CompactTest::pair(                                             0b00100000'00000000'00000000, {0b11100000, 0b00000000, 0b00000000, 0b00100000}),
  /*  4 */ CompactTest::pair(                                    0b00010000'00000000'00000000'00000000, {0b11110000, 0b00000000, 0b00000000, 0b00000000, 0b00010000}),
  /*  5 */ CompactTest::pair(                           0b00001000'00000000'00000000'00000000'00000000, {0b11111000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00001000}),
  /*  6 */ CompactTest::pair(                   0b0000100'00000000'00000000'00000000'00000000'00000000, {0b11111100, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000100}),
  /*  7 */ CompactTest::pair(          0b0000010'00000000'00000000'00000000'00000000'00000000'00000000, {0b11111110, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000010}),
  /*  8 */ CompactTest::pair(0b00000001'00000000'00000000'00000000'00000000'00000000'00000000'00000000, {0b11111111, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000001}),

        // Biggest values for each number of bytes
  /*  9 */ CompactTest::pair(                                                               0b01111111, {0b01111111}),
  /* 10 */ CompactTest::pair(                                                      0b00111111'11111111, {0b10111111, 0b11111111}),
  /* 11 */ CompactTest::pair(                                             0b00011111'11111111'11111111, {0b11011111, 0b11111111, 0b11111111}),
  /* 12 */ CompactTest::pair(                                    0b00001111'11111111'11111111'11111111, {0b11101111, 0b11111111, 0b11111111, 0b11111111}),
  /* 13 */ CompactTest::pair(                           0b00000111'11111111'11111111'11111111'11111111, {0b11110111, 0b11111111, 0b11111111, 0b11111111, 0b11111111}),
  /* 14 */ CompactTest::pair(                  0b00000011'11111111'11111111'11111111'11111111'11111111, {0b11111011, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111}),
  /* 15 */ CompactTest::pair(         0b00000001'11111111'11111111'11111111'11111111'11111111'11111111, {0b11111101, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111}),
  /* 16 */ CompactTest::pair(0b00000000'11111111'11111111'11111111'11111111'11111111'11111111'11111111, {0b11111110, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111}),
  /* 17 */ CompactTest::pair(0b11111111'11111111'11111111'11111111'11111111'11111111'11111111'11111111, {0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111})
        // clang-format on
        ));

#else

#define BIGGEST_INT_FOR_COMPACT_REPRESENTATION \
  (CompactInteger(1) << (67 * CHAR_WIDTH)) - 1

INSTANTIATE_TEST_SUITE_P(
    CompactTestCases,
    CompactTest,
    ::testing::Values(
        // 0 is min compact integer value, negative values are not allowed
        CompactTest::pair(0, {0}),
        // 1 is encoded as 4
        CompactTest::pair(1, {4}),
        // max 1 byte value
        CompactTest::pair(63, {252}),
        // min 2 bytes value
        CompactTest::pair(64, {1, 1}),
        // some 2 bytes value
        CompactTest::pair(255, {253, 3}),
        // some 2 bytes value
        CompactTest::pair(511, {253, 7}),
        // max 2 bytes value
        CompactTest::pair(16383, {253, 255}),
        // min 4 bytes value
        CompactTest::pair(16384, {2, 0, 1, 0}),
        // some 4 bytes value
        CompactTest::pair(65535, {254, 255, 3, 0}),
        // max 4 bytes value
        CompactTest::pair(1073741823ul, {254, 255, 255, 255}),
        // some multibyte integer
        CompactTest::pair(
            CompactInteger("1234567890123456789012345678901234567890"),
            {0b110111,
             210,
             10,
             63,
             206,
             150,
             95,
             188,
             172,
             184,
             243,
             219,
             192,
             117,
             32,
             201,
             160,
             3}),
        // min multibyte integer
        CompactTest::pair(1073741824, {3, 0, 0, 0, 64}),
        // max multibyte integer:  2^536 - 1
        CompactTest::pair(BIGGEST_INT_FOR_COMPACT_REPRESENTATION,
                          std::vector<uint8_t>(68, 0xFF))));

#endif

/**
 * Negative tests
 */

/**
 * @given a negative value -1
 * (negative values are not supported by compact encoding)
 * @when trying to encode this value
 * @then obtain error
 */
TEST(ScaleCompactTest, EncodeNegativeIntegerFails) {
  CompactInteger v(-1);
  ScaleEncoderStream out{};
  ASSERT_ANY_THROW((out << v));
  ASSERT_EQ(out.to_vector().size(), 0);  // nothing was written to buffer
}

/**
 * @given a CompactInteger value exceeding the range supported by scale
 * @when encode it a directly as CompactInteger
 * @then obtain kValueIsTooBig error
 */
TEST(ScaleCompactTest, EncodeOutOfRangeBigIntegerFails) {
  // try to encode out of range big integer value MAX_BIGINT + 1
  // too big value, even for big integer case
  // we are going to have kValueIsTooBig error
  CompactInteger v = BIGGEST_INT_FOR_COMPACT_REPRESENTATION + 1;

  ScaleEncoderStream out;
  ASSERT_ANY_THROW((out << v));          // value is too big, it isn't encoded
  ASSERT_EQ(out.to_vector().size(), 0);  // nothing was written to buffer
}

/**
 * @given incorrect byte array, which assumes 4-th case of encoding
 * @when apply decodeInteger
 * @then get kNotEnoughData error
 */
TEST(Scale, compactDecodeBigIntegerError) {
  auto bytes = ByteArray{255, 255, 255, 255};
  EXPECT_EC(decode<CompactInteger>(bytes), scale::DecodeError::NOT_ENOUGH_DATA);
}

/**
 * @given redundant bytes in compact encoding
 * @when decode compact
 * @then error
 */
struct RedundantCompactTest : ::testing::TestWithParam<ByteArray> {};
TEST_P(RedundantCompactTest, DecodeError) {
  EXPECT_EC(scale::decode<CompactInteger>(GetParam()),
            scale::DecodeError::REDUNDANT_COMPACT_ENCODING);
}

#ifdef JAM_COMPATIBILITY_ENABLED

INSTANTIATE_TEST_SUITE_P(
    RedundantCompactTestCases,
    RedundantCompactTest,
    ::testing::Values(
        // clang-format off
  /*  1 */ ByteArray{0b10000000, 0b00000000},
  /*  2 */ ByteArray{0b10000000, 0b00111111},
  /*  3 */ ByteArray{0b11000000, 0b00000000, 0b00000000},
  /*  4 */ ByteArray{0b11000000, 0b11111111, 0b00011111},
  /*  5 */ ByteArray{0b11100000, 0b00000000, 0b00000000, 0b00000000},
  /*  6 */ ByteArray{0b11100000, 0b11111111, 0b11111111, 0b00001111},
  /*  7 */ ByteArray{0b11110000, 0b00000000, 0b00000000, 0b00000000, 0b00000000},
  /*  8 */ ByteArray{0b11110000, 0b11111111, 0b11111111, 0b11111111, 0b00000111},
  /*  9 */ ByteArray{0b11111000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000},
  /* 10 */ ByteArray{0b11111000, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b00000011},
  /* 11 */ ByteArray{0b11111100, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000},
  /* 12 */ ByteArray{0b11111100, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b00000001},
  /* 13 */ ByteArray{0b11111110, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000},
  /* 14 */ ByteArray{0b11111110, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b00000000},
  /* 15 */ ByteArray{0b11111111, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000},
  /* 16 */ ByteArray{0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b00000000}
     // // clang-format on
        ));

#else

INSTANTIATE_TEST_SUITE_P(
    RedundantCompactTestCases,
    RedundantCompactTest,
    ::testing::Values(ByteArray{0b100000'01, 0},
                      ByteArray{0b000000'10, 0b10000000, 0, 0},
                      ByteArray{0b000000'11, 0, 0, 0, 0b00'100000},
                      ByteArray{0b000001'11, 0, 0, 0, 0b01'000000, 0}));

#endif

