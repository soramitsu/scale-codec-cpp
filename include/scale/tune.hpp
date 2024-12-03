/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

namespace scale::detail {

  enum Compatibility {
    Classic,
    Jam,
  };
}

namespace scale::tune {
  inline static constexpr detail::Compatibility classic =
      detail::Compatibility::Classic;
  inline static constexpr detail::Compatibility jam_compatible =
      detail::Compatibility::Jam;

}  // namespace scale::tune
