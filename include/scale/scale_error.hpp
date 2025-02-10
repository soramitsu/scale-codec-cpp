/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <qtils/enum_error_code.hpp>

namespace scale {

  /**
   * @brief EncodeError enum provides error codes for Encode methods
   */
  enum class EncodeError {
    /// negative integers is not supported
    NEGATIVE_INTEGER,
    /// dereferencing a null pointer
    DEREF_NULLPOINTER,
    /// value too bit for compact representation
    VALUE_TOO_BIG_FOR_COMPACT_REPRESENTATION,
  };

  /**
   * @brief DecoderError enum provides codes of errors for Decoder methods
   */
  enum class DecodeError {
    NOT_ENOUGH_DATA = 1,  ///< not enough data to decode value
    UNEXPECTED_VALUE,     ///< unexpected value
    TOO_MANY_ITEMS,       ///< too many items, cannot address them in memory
    WRONG_TYPE_INDEX,     ///< wrong type index, cannot decode variant
    INVALID_ENUM_VALUE,   ///< enum value which doesn't belong to the enum
    REDUNDANT_COMPACT_ENCODING,      ///< redundant bytes in compact encoding
    DECODED_VALUE_OVERFLOWS_TARGET,  ///< encoded value overflows target type
  };

}  // namespace scale

OUTCOME_HPP_DECLARE_ERROR(scale, EncodeError)
OUTCOME_HPP_DECLARE_ERROR(scale, DecodeError)
