/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CUSTOM_CONFIG_ENABLED
#error \
    "This file should not be compiled, because custom config support is not enabed"
#endif

#include <gtest/gtest.h>

#include <scale/scale.hpp>

using scale::ByteArray;
using scale::CompactInteger;
using scale::decode;
using scale::encode;
using scale::ScaleDecoderStream;
using scale::ScaleEncoderStream;

struct CustomConfig {
  uint8_t multi;
};

struct Object {
  std::vector<uint8_t> buff;

  bool operator==(const Object &) const = default;

  friend ScaleEncoderStream &operator<<(ScaleEncoderStream &s,
                                        const Object &x) {
    auto config = s.getConfig<CustomConfig>();

    s << CompactInteger{x.buff.size()};
    for (uint8_t i : x.buff) {
      uint8_t x = i * config.multi;
      s << x;
    }
    return s;
  }

  friend ScaleDecoderStream &operator>>(ScaleDecoderStream &s, Object &x) {
    auto config = s.getConfig<CustomConfig>();
    CompactInteger size;
    s >> size;
    x.buff.resize(size.convert_to<size_t>());
    for (uint8_t &i : x.buff) {
      uint8_t x;
      s >> x;
      i = x / config.multi;
    }
    return s;
  }
};

TEST(CustomConfig, SunnyDayScenario) {
  CustomConfig two{2};
  CustomConfig three{3};

  // Encoding
  Object object{.buff = {3, 6, 9}};
  ScaleEncoderStream encoder(two);
  encoder << object;
  auto encoded = encoder.to_vector();

  // Check encoding
  ScaleDecoderStream s(encoded);
  std::vector<uint8_t> buff1;
  s >> buff1;
  std::vector<uint8_t> buff2 = {6, 12, 18};
  ASSERT_EQ(buff1, buff2) << "Incorrect encoded";

  // Decode and check it
  ScaleDecoderStream decoder(encoded, three);
  Object object1;
  decoder >> object1;
  Object object2{.buff = {2, 4, 6}};
  ASSERT_EQ(object1, object2) << "Incorrect decoded";
}
