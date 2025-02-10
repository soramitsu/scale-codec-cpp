/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include <scale/definitions.hpp>
#include <scale/scale.hpp>

#ifndef CUSTOM_CONFIG_ENABLED
#error \
    "This file should not be compiled, because custom config support is not enabed"
#endif

using scale::ByteArray;
using scale::Compact;
using scale::decode;
using scale::encode;
using scale::Length;
using scale::ScaleDecoderStream;
using scale::ScaleEncoderStream;

struct MulConfig {
  uint8_t multi;
};
struct AddConfig {
  uint8_t add;
};

struct Object {
  std::vector<uint8_t> buff;

  bool operator==(const Object &) const = default;

  friend ScaleEncoderStream &operator<<(ScaleEncoderStream &s,
                                        const Object &x) {
    auto mul = s.getConfig<MulConfig>().multi;
    auto add = s.getConfig<AddConfig>().add;

    s << Length(x.buff.size());
    for (uint8_t i : x.buff) {
      uint8_t x = i * mul + add;
      s << x;
    }
    return s;
  }

  friend ScaleDecoderStream &operator>>(ScaleDecoderStream &s, Object &x) {
    auto mul = s.getConfig<MulConfig>().multi;
    auto add = s.getConfig<AddConfig>().add;
    Length size;
    s >> size;
    x.buff.resize(untagged(size));
    for (uint8_t &i : x.buff) {
      uint8_t x;
      s >> x;
      i = (x - add) / mul;
    }
    return s;
  }
};

TEST(CustomConfig, SunnyDayScenario) {
  MulConfig mul_two{2};
  MulConfig mul_three{3};
  AddConfig add_six{6};
  AddConfig add_twelve{12};

  // Encoding
  Object object{.buff = {3, 6, 9}};
  ScaleEncoderStream encoder(mul_two, add_twelve);
  encoder << object;  // X * 2 + 12
  auto encoded = encoder.to_vector();

  // Check encoding
  ScaleDecoderStream s(encoded);
  std::vector<uint8_t> buff1;
  s >> buff1;
  std::vector<uint8_t> buff2 = {18, 24, 30};
  ASSERT_EQ(buff1, buff2) << "Incorrect encoded";

  // Decode and check it
  ScaleDecoderStream decoder(encoded, mul_three, add_six);
  Object object1;
  decoder >> object1;
  Object object2{.buff = {4, 6, 8}};
  ASSERT_EQ(object1, object2) << "Incorrect decoded";
}
