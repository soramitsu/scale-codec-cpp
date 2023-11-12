/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "scale/scale_error.hpp"

OUTCOME_CPP_DEFINE_CATEGORY(scale, EncodeError, e) {
  using scale::EncodeError;
  switch (e) {
    case EncodeError::NEGATIVE_COMPACT_INTEGER:
      return "SCALE encode: compact integers cannot be negative";
    case EncodeError::COMPACT_INTEGER_TOO_BIG:
      return "SCALE encode: compact integers too big";
    case EncodeError::DEREF_NULLPOINTER:
      return "SCALE encode: attempt to dereference a nullptr";
  }
  return "unknown EncodeError";
}

OUTCOME_CPP_DEFINE_CATEGORY(scale, DecodeError, e) {
  using scale::DecodeError;
  switch (e) {
    case DecodeError::NOT_ENOUGH_DATA:
      return "SCALE decode: not enough data to decode";
    case DecodeError::UNEXPECTED_VALUE:
      return "SCALE decode: unexpected value occurred";
    case DecodeError::TOO_MANY_ITEMS:
      return "SCALE decode: collection has too many items or memory is out or "
             "data is damaged, unable to unpack";
    case DecodeError::WRONG_TYPE_INDEX:
      return "SCALE decode: wrong type index, cannot decode variant";
    case DecodeError::INVALID_ENUM_VALUE:
      return "SCALE decode: decoded enum value does not belong to the enum";
  }
  return "unknown SCALE DecodeError";
}
