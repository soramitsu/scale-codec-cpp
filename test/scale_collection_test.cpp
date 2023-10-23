/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include "scale/scale.hpp"
#include "util/outcome.hpp"

using scale::BitVec;
using scale::ByteArray;
using scale::CompactInteger;
using scale::decode;
using scale::DecodeError;
using scale::encode;
using scale::ScaleDecoderStream;
using scale::ScaleEncoderStream;

/**
 * @given collection of 80 items of type uint8_t
 * @when encodeCollection is applied
 * @then expected result is obtained: header is 2 byte, items are 1 byte each
 */
TEST(Scale, encodeCollectionOf80) {
  // 80 items of value 1
  ByteArray collection(80, 1);
  auto match = ByteArray{65, 1};  // header
  match.insert(match.end(), collection.begin(), collection.end());
  ScaleEncoderStream s;
  ASSERT_NO_THROW((s << collection));
  auto &&out = s.to_vector();
  ASSERT_EQ(out.size(), 82);
  ASSERT_EQ(out, match);
}

/**
 * @given vector of bools
 * @when encodeCollection is applied
 * @then expected result is obtained: header is 2 byte, items are 1 byte each
 */
TEST(Scale, encodeVectorOfBool) {
  std::vector<bool> collection = {true, false, true, false, false, false};
  ScaleEncoderStream s;
  ASSERT_NO_THROW((s << collection));
  auto &&out = s.to_vector();

  auto stream = ScaleDecoderStream(out);
  std::vector<bool> decoded;
  stream >> decoded;
  ASSERT_TRUE(std::equal(
      decoded.begin(), decoded.end(), collection.begin(), collection.end()));

  // clang-format off
  ASSERT_EQ(out,
          (ByteArray{
            24, // header
            1,  // first item
            0,  // second item
            1,  // third item
            0,  // fourth item
            0,  // fifth item
            0   // sixths item
              }));
  // clang-format on
}

TEST(Scale, encodeBitVec) {
  auto v = BitVec{{true, true, false, false, false, false, true}};
  auto encoded = ByteArray{(7 << 2), 0b01000011};
  ASSERT_EQ(encode(v).value(), encoded);
  ASSERT_EQ(decode<BitVec>(encoded).value(), v);
}

/**
 * @given collection of items of type uint16_t
 * @when encodeCollection is applied
 * @then expected result is obtained
 */
TEST(Scale, encodeCollectionUint16) {
  std::vector<uint16_t> collection = {1, 2, 3, 4};
  ScaleEncoderStream s;
  ASSERT_NO_THROW((s << collection));
  auto &&out = s.to_vector();

  auto stream = ScaleDecoderStream(out);
  std::vector<uint16_t> decoded;
  stream >> decoded;
  ASSERT_TRUE(std::equal(
      decoded.begin(), decoded.end(), collection.begin(), collection.end()));

  // clang-format off
  ASSERT_EQ(out,
          (ByteArray{
              16,  // header
            1, 0,  // first item
            2, 0,  // second item
            3, 0,  // third item
            4, 0  // fourth item
              }));
  // clang-format on
}

struct TestStruct : std::vector<uint16_t> {
  static constexpr bool is_static_collection = false;
};

/**
 * @given collection of items of type uint16_t, derived from std::vector
 * @when encodeCollection is applied
 * @then expected result is obtained
 */
TEST(Scale, encodeDerivedCollectionUint16) {
  TestStruct collection;
  collection.push_back(1);
  collection.push_back(2);
  collection.push_back(3);
  collection.push_back(4);

  ScaleEncoderStream s;
  ASSERT_NO_THROW((s << collection));
  auto &&out = s.to_vector();

  auto stream = ScaleDecoderStream(out);
  TestStruct decoded;
  stream >> decoded;
  ASSERT_TRUE(std::equal(
      decoded.begin(), decoded.end(), collection.begin(), collection.end()));

  // clang-format off
  ASSERT_EQ(out,
          (ByteArray{
              16,  // header
            1, 0,  // first item
            2, 0,  // second item
            3, 0,  // third item
            4, 0  // fourth item
              }));
  // clang-format on
}

/**
 * @given collection of items of type uint16_t
 * @when encodeCollection is applied
 * @then expected result is obtained
 */
TEST(Scale, encodeDequeUint16) {
  std::deque<uint16_t> collection = {1, 2, 3, 4};
  ScaleEncoderStream s;
  ASSERT_NO_THROW((s << collection));
  auto &&out = s.to_vector();

  auto stream = ScaleDecoderStream(out);
  std::deque<uint16_t> decoded;
  stream >> decoded;
  ASSERT_TRUE(std::equal(
      decoded.begin(), decoded.end(), collection.begin(), collection.end()));

  // clang-format off
  ASSERT_EQ(out,
          (ByteArray{
              16,  // header
            1, 0,  // first item
            2, 0,  // second item
            3, 0,  // third item
            4, 0  // fourth item
              }));
  // clang-format on
}

/**
 * @given collection of items of type uint32_t
 * @when encodeCollection is applied
 * @then expected result is obtained
 */
TEST(Scale, encodeCollectionUint32) {
  std::vector<uint32_t> collection = {
      50462976, 117835012, 185207048, 252579084};
  ScaleEncoderStream s;
  ASSERT_NO_THROW((s << collection));
  auto &&out = s.to_vector();
  // clang-format off
  ASSERT_EQ(out,
            (ByteArray{
                    16,                // header
                    0, 1, 2, 3,        // first item
                    4, 5, 6, 7,        // second item
                    8, 9, 0xA, 0xB,    // third item
                    0xC, 0xD, 0xE, 0xF // fourth item
            }));
  // clang-format on
}

/**
 * @given collection of items of type uint64_t
 * @when encodeCollection is applied
 * @then expected result is obtained
 */
TEST(Scale, encodeCollectionUint64) {
  std::vector<uint64_t> collection = {506097522914230528ull,
                                      1084818905618843912ull};
  ScaleEncoderStream s;
  ASSERT_NO_THROW((s << collection));
  auto &&out = s.to_vector();
  // clang-format off
  ASSERT_EQ(out,
            (ByteArray{
                    8,                // header
                    0, 1, 2, 3,        // first item
                    4, 5, 6, 7,        // second item
                    8, 9, 0xA, 0xB,    // third item
                    0xC, 0xD, 0xE, 0xF // fourth item
            }));
  // clang-format on
}

/**
 * @given collection of items of type uint16_t containing 2^14 items
 * where collection[i]  == i % 256
 * @when encodeCollection is applied
 * @then obtain byte array of length 32772 bytes
 * where each second byte == 0 and collection[(i-4)/2] == (i/2) % 256
 */
TEST(Scale, encodeLongCollectionUint16) {
  std::vector<uint16_t> collection;
  auto length = 16384;
  collection.reserve(length);
  for (auto i = 0; i < length; ++i) {
    collection.push_back(i % 256);
  }

  ScaleEncoderStream s;
  ASSERT_NO_THROW((s << collection));
  auto &&out = s.to_vector();
  ASSERT_EQ(out.size(), (length * 2 + 4));

  // header takes 4 byte,
  // first 4 bytes represent le-encoded value 2^16 + 2
  // which is compact-encoded value 2^14 = 16384
  auto stream = ScaleDecoderStream(out);
  CompactInteger res{};
  ASSERT_NO_THROW(stream >> res);
  ASSERT_EQ(res, 16384);

  // now only 32768 bytes left in stream
  ASSERT_EQ(stream.hasMore(32768), true);
  ASSERT_EQ(stream.hasMore(32769), false);

  for (auto i = 0; i < length; ++i) {
    uint8_t byte = 0u;
    ASSERT_NO_THROW(stream >> byte);
    ASSERT_EQ(byte, i % 256);
    ASSERT_NO_THROW(stream >> byte);
    ASSERT_EQ(byte, 0);
  }

  ASSERT_EQ(stream.hasMore(1), false);
}

/**
 * @given very long collection of items of type uint8_t containing 2^20 items
 * this number takes ~ 1 Mb of data
 * where collection[i]  == i % 256
 * @when encodeCollection is applied
 * @then obtain byte array of length 1048576 + 4 bytes (header) bytes
 * where first bytes repreent header, other are data itself
 * where each byte after header == i%256
 */

TEST(Scale, encodeVeryLongCollectionUint8) {
  auto length = 1048576;  // 2^20
  std::vector<uint8_t> collection;
  collection.reserve(length);

  for (auto i = 0; i < length; ++i) {
    collection.push_back(i % 256);
  }

  ScaleEncoderStream s;
  ASSERT_NO_THROW((s << collection));
  auto &&out = s.to_vector();
  ASSERT_EQ(out.size(), (length + 4));
  // header takes 4 bytes,
  // first byte == (4-4) + 3 = 3,
  // which means that number of items requires 4 bytes
  // 3 next bytes are 0, and the last 4-th == 2^6 == 64
  // which is compact-encoded value 2^14 = 16384
  auto stream = ScaleDecoderStream(out);
  CompactInteger bi{};
  ASSERT_NO_THROW(stream >> bi);
  ASSERT_EQ(bi, 1048576);

  // now only 1048576 bytes left in stream
  ASSERT_EQ(stream.hasMore(1048576), true);
  ASSERT_EQ(stream.hasMore(1048576 + 1), false);

  for (auto i = 0; i < length; ++i) {
    uint8_t byte{0u};
    ASSERT_NO_THROW((stream >> byte));
    ASSERT_EQ(byte, i % 256);
  }

  ASSERT_EQ(stream.hasMore(1), false);
}

// following test takes too much time, don't run it
/**
 * @given very long collection of items of type uint8_t containing 2^30 ==
 * 1073741824 items this number takes ~ 1 Gb of data where collection[i]  == i %
 * 256
 * @when encodeCollection is applied
 * @then obtain byte array of length 1073741824 + 5 bytes (header) bytes
 * where first bytes represent header, other are data itself
 * where each byte after header == i%256
 */
TEST(Scale, DISABLED_encodeVeryLongCollectionUint8) {
  auto length = 1073741824;  // 2^20
  std::vector<uint8_t> collection;

  collection.reserve(length);
  for (auto i = 0; i < length; ++i) {
    collection.push_back(i % 256);
  }

  ScaleEncoderStream s;
  ASSERT_NO_THROW((s << collection));
  auto &&out = s.to_vector();
  ASSERT_EQ(out.size(), (length + 4));
  // header takes 4 bytes,
  // first byte == (4-4) + 3 = 3, which means that number of items
  // requires 4 bytes
  // 3 next bytes are 0, and the last 4-th == 2^6 == 64
  // which is compact-encoded value 2^14 = 16384
  auto stream = ScaleDecoderStream(out);
  CompactInteger bi{};
  ASSERT_NO_THROW(stream >> bi);
  ASSERT_EQ(bi, length);

  // now only 1048576 bytes left in stream
  ASSERT_EQ(stream.hasMore(length), true);
  ASSERT_EQ(stream.hasMore(length + 1), false);

  for (auto i = 0; i < length; ++i) {
    uint8_t byte = 0u;
    ASSERT_NO_THROW(stream >> byte);
    ASSERT_EQ(byte, i % 256);
  }

  ASSERT_EQ(stream.hasMore(1), false);
}

/**
 * @given map of <uint32_t, uint32_t>
 * @when encodeCollection is applied
 * @then expected result is obtained: header is 2 byte, items are pairs of 4
 * byte elements each
 */
TEST(Scale, encodeMapTest) {
  std::map<uint32_t, uint32_t> collection = {{1, 5}, {2, 6}, {3, 7}, {4, 8}};
  ScaleEncoderStream s;
  ASSERT_NO_THROW((s << collection));
  auto &&out = s.to_vector();

  auto stream = ScaleDecoderStream(out);
  std::map<uint32_t, uint32_t> decoded;
  stream >> decoded;
  ASSERT_TRUE(std::equal(
      decoded.begin(), decoded.end(), collection.begin(), collection.end()));
}

template <template <typename...> class BaseContainer,
          size_t WithMaxSize,
          typename... Args>
class SizeLimitedContainer : public BaseContainer<Args...> {
  using Base = BaseContainer<Args...>;

 public:
  static constexpr bool is_static_collection = false;
  using Base::Base;
  using typename Base::size_type;

  size_type max_size() const {
    return WithMaxSize;
  }
};

template <size_t WithMaxSize, typename... Args>
using SizeLimitedVector =
    SizeLimitedContainer<std::vector, WithMaxSize, Args...>;

/**
 * @given encoded 3-elements collection
 * @when decode it to collection limited by size 4, 3 and 2 max size
 * @then if max_size is enough, it is done successful, and error otherwise
 */
TEST(Scale, decodeSizeLimitedCollection) {
  std::vector<int> collection{1, 2, 3};

  ScaleEncoderStream s;
  ASSERT_NO_THROW((s << collection));
  auto &&out = s.to_vector();

  {
    auto stream = ScaleDecoderStream(out);
    SizeLimitedVector<4, int> decoded;
    ASSERT_NO_THROW((stream >> decoded));
    ASSERT_TRUE(std::equal(
        decoded.begin(), decoded.end(), collection.begin(), collection.end()));
  }
  {
    auto stream = ScaleDecoderStream(out);
    SizeLimitedVector<3, int> decoded;
    ASSERT_NO_THROW((stream >> decoded));
    ASSERT_TRUE(std::equal(
        decoded.begin(), decoded.end(), collection.begin(), collection.end()));
  }
  {
    auto stream = ScaleDecoderStream(out);
    SizeLimitedVector<2, int> decoded;

    try {
      stream >> decoded;
      FAIL() << "Exception expected";
    } catch (std::system_error &e) {
      EXPECT_EQ(e.code(), DecodeError::TOO_MANY_ITEMS);
    }
  }
}

struct ExplicitlyDefinedAsStatic : public std::vector<int> {
  static constexpr bool is_static_collection = true;
  using Collection = std::vector<int>;
  using Collection::Collection;
};

struct ExplicitlyDefinedAsDynamic : public std::vector<int> {
  static constexpr bool is_static_collection = false;
  using Collection = std::vector<int>;
  using Collection::Collection;
};

struct ImplicitlyDefinedAsStatic : public std::array<int, 5> {
  //  using Collection = std::array<int, 5>;
  //  using Collection::Collection;
};

struct ImplicitlyDefinedAsDynamic : public std::vector<int> {
  using Collection = std::vector<int>;
  using Collection::Collection;
};

TEST(Scale, encodeExplicitlyDefinedAsStatic) {
  using TestCollection = ExplicitlyDefinedAsStatic;

  const TestCollection collection{1, 2, 3, 4, 5};

  ScaleEncoderStream s;
  ASSERT_NO_THROW((s << collection));
  auto &&out = s.to_vector();

  auto stream = ScaleDecoderStream(out);
  TestCollection decoded{0xff, 0xff, 0xff, 0xff, 0xff};
  stream >> decoded;
  ASSERT_TRUE(std::equal(
      decoded.begin(), decoded.end(), collection.begin(), collection.end()));
}

TEST(Scale, encodeExplicitlyDefinedAsDynamic) {
  using TestCollection = ExplicitlyDefinedAsDynamic;

  const TestCollection collection{1, 2, 3, 4, 5};
  ScaleEncoderStream s;
  ASSERT_NO_THROW((s << collection));
  auto &&out = s.to_vector();

  auto stream = ScaleDecoderStream(out);
  TestCollection decoded{0xff, 0xff, 0xff};
  stream >> decoded;
  ASSERT_TRUE(std::equal(
      decoded.begin(), decoded.end(), collection.begin(), collection.end()));
}

TEST(Scale, encodeImplicitlyDefinedAsStatic) {
  using TestCollection = ImplicitlyDefinedAsStatic;

  const TestCollection collection{1, 2, 3, 4, 5};
  ScaleEncoderStream s;
  ASSERT_NO_THROW((s << collection));
  auto &&out = s.to_vector();

  auto stream = ScaleDecoderStream(out);
  TestCollection decoded{0xff, 0xff, 0xff, 0xff, 0xff};
  stream >> decoded;
  ASSERT_TRUE(std::equal(
      decoded.begin(), decoded.end(), collection.begin(), collection.end()));
}

TEST(Scale, encodeImplicitlyDefinedAsDynamic) {
  using TestCollection = ImplicitlyDefinedAsDynamic;

  const TestCollection collection{1, 2, 3, 4, 5};
  ScaleEncoderStream s;
  ASSERT_NO_THROW((s << collection));
  auto &&out = s.to_vector();

  auto stream = ScaleDecoderStream(out);
  TestCollection decoded{0xff, 0xff, 0xff};
  stream >> decoded;
  ASSERT_TRUE(std::equal(
      decoded.begin(), decoded.end(), collection.begin(), collection.end()));
}
