/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <boost/throw_exception.hpp>
#include <system_error>

namespace scale {
  [[noreturn]] inline void raise(const std::error_code &ec) {
    boost::throw_exception(std::system_error(ec));
  }
}  // namespace scale
