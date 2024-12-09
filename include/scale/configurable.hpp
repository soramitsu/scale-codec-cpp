/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifdef CUSTOM_CONFIG_ENABLED
#include <any>
#endif

namespace scale {

  class Configurable {
   public:
    Configurable() = default;
    ~Configurable() = default;

    template <typename T>
    explicit Configurable(const T &config) : config_(std::cref(config)) {}

#ifdef CUSTOM_CONFIG_ENABLED
    template <typename T>
    const T &getConfig() const {
      if (not config_.has_value()) {
        throw std::runtime_error("Stream created without any custom config");
      }
      if (config_.type() != typeid(std::reference_wrapper<const T>)) {
        throw std::runtime_error(
            "Stream created with other custom config type");
      }
      return std::any_cast<std::reference_wrapper<const T>>(config_).get();
    }
#else
    template <typename T>
    [[deprecated("Scale has compiled without custom config support")]]  //
    const T &
    getConfig() const = delete;
#endif

#ifdef CUSTOM_CONFIG_ENABLED
    const std::any config_{};
#endif
  };

}  // namespace scale