/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <scale/outcome/outcome.hpp>
#include <type_traits>

namespace scale {
  template <typename F>
  outcome::result<std::invoke_result_t<F>> outcomeCatch(F &&f) {
    try {
      if constexpr (std::is_void_v<std::invoke_result_t<F>>) {
        f();
        return outcome::success();
      } else {
        return outcome::success(f());
      }
    } catch (std::system_error &e) {
      return outcome::failure(e.code());
    }
  }
}  // namespace scale
