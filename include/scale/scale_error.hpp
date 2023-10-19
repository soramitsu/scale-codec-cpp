/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <scale/outcome/outcome.hpp>

#include <scale/types.hpp>

namespace scale {

  /**
   * @brief EncodeError enum provides error codes for Encode methods
   */
  enum class EncodeError {
    COMPACT_INTEGER_TOO_BIG = 1,  ///< compact integer can't be more than 2**536
    NEGATIVE_COMPACT_INTEGER,     ///< cannot compact-encode negative integers
    DEREF_NULLPOINTER,            ///< dereferencing a null pointer
  };

  /**
   * @brief DecoderError enum provides codes of errors for Decoder methods
   */
  enum class DecodeError {
    NOT_ENOUGH_DATA = 1,  ///< not enough data to decode value
    UNEXPECTED_VALUE,     ///< unexpected value
    TOO_MANY_ITEMS,       ///< too many items, cannot address them in memory
    WRONG_TYPE_INDEX,     ///< wrong type index, cannot decode variant
    INVALID_ENUM_VALUE    ///< enum value which doesn't belong to the enum
  };

}  // namespace scale

OUTCOME_HPP_DECLARE_ERROR_2(scale, EncodeError)
OUTCOME_HPP_DECLARE_ERROR_2(scale, DecodeError)
