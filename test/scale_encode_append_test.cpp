/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "append_test_data.hpp"
#include "scale/encode_append.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <boost/algorithm/hex.hpp>

using ::testing::ContainerEq;

namespace {
  std::vector<uint8_t> unhex(std::string_view hex) {
    std::vector<uint8_t> blob;
    blob.reserve((hex.size() + 1) / 2);

    boost::algorithm::unhex(hex.begin(), hex.end(), std::back_inserter(blob));
    return blob;
  }
}  // namespace

namespace scale {

  TEST(EncodeAppend, Append) {
    auto inp1 = scale::encode(std::vector<uint32_t>({1, 2, 3, 4, 5})).value();
    EncodeOpaqueValue inp1_eov{inp1};

    std::vector<uint8_t> res{};
    auto inp1_encoded = scale::encode(inp1_eov).value();
    auto encoded_concat_res = append_or_new_vec(res, inp1_encoded);

    ASSERT_TRUE(encoded_concat_res.has_value());
    // vector below obtained by debugging substrate
    ASSERT_THAT(res, ContainerEq(std::vector<uint8_t>({4, 20, 1, 0, 0, 0, 2, 0,
                                                       0, 0,  3, 0, 0, 0, 4, 0,
                                                       0, 0,  5, 0, 0, 0})));

    auto inp2 = scale::encode(uint32_t{2}).value();
    EncodeOpaqueValue inp2_eov{inp2};
    auto inp2_encoded = scale::encode(inp2_eov).value();
    encoded_concat_res = append_or_new_vec(res, inp2_encoded);

    ASSERT_TRUE(encoded_concat_res.has_value());
    // vector below obtained by debugging substrate
    ASSERT_THAT(res,
                ContainerEq(std::vector<uint8_t>({8, 20, 1, 0, 0, 0, 2, 0, 0,
                                                  0, 3,  0, 0, 0, 4, 0, 0, 0,
                                                  5, 0,  0, 0, 2, 0, 0, 0})));
  }

  TEST(EncodeAppend, HugeBlob) {
    auto val = unhex(data::val);
    auto append_bytes = unhex(data::append_bytes);

    auto res = append_or_new_vec(val, append_bytes);
    ASSERT_TRUE(res.has_value());
  }
}  // namespace scale
