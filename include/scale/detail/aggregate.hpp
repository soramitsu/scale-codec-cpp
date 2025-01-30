/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <scale/types.hpp>

namespace scale::detail {

  template <typename F>
  decltype(auto) decompose_and_apply(SimpleCodeableAggregate auto &&v,
                                     const F &f) {
    constexpr auto N = field_number_of<decltype(v)>;
    // clang-format off
    if constexpr (N == 0) {
      return f();
    // -BEGIN-GENERATED-SECTION-
    // -END-GENERATED-SECTION-
    } else {
      // We mustn't fall in here
      static_assert(N <= MAX_FIELD_NUM, "Inconsistent value of MAX_FIELD_NUM");
      static_assert(N > MAX_FIELD_NUM, "No code for cover aggregate with such big amount of fields");
    }
    // clang-format on
  }

}  // namespace scale::detail
