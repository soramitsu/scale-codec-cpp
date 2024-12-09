/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifdef CUSTOM_CONFIG_ENABLED
#include <any>
#endif
#include <type_traits>
#include <typeindex>
#include <unordered_map>

namespace scale {

  template <typename T>
  concept MaybeConfig = std::is_class_v<T> and not std::is_union_v<T>;

  class Configurable {
   public:
    Configurable() = default;
    ~Configurable() = default;

#ifdef CUSTOM_CONFIG_ENABLED
    template <typename... ConfigTs>
      requires(MaybeConfig<ConfigTs> and ...)
    explicit Configurable(const ConfigTs &...configs) {
      (addConfig(configs), ...);
    }
#else
    template <typename... ConfigTs>
      requires(MaybeConfig<ConfigTs> and ...)
    explicit Configurable(const ConfigTs &...configs) {}
#endif

#ifdef CUSTOM_CONFIG_ENABLED
    template <typename T>
      requires MaybeConfig<T>
    const T &getConfig() const {
      const auto it = configs_.find(typeid(T));
      if (it == configs_.end()) {
        throw std::runtime_error(
            "Stream was not configured by such custom config type");
      }
      return std::any_cast<std::reference_wrapper<const T>>(it->second).get();
    }
#else
    template <typename T>
    [[deprecated("Scale has compiled without custom config support")]]  //
    const T &
    getConfig() const = delete;
#endif

#ifdef CUSTOM_CONFIG_ENABLED
   private:
    template <typename ConfigT>
    void addConfig(const ConfigT &config) {
      auto [_, added] = configs_.emplace(typeid(ConfigT), std::cref(config));
      if (not added) {
        throw std::runtime_error(
            "Stream can be configured by different custom config types only");
      }
    }

    std::unordered_map<std::type_index, std::any> configs_{};
#endif
  };

}  // namespace scale