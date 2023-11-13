/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <gtest/gtest.h>

#define EXPECT_OUTCOME_TRUE_name(var, val, expr)                            \
  auto &&var = expr;                                                        \
  EXPECT_TRUE(var) << "Line " << __LINE__ << ": " << var.error().message(); \
  auto &&val = var.value();

/**
 * Use this macro in GTEST with 2 arguments to assert that getResult()
 * returned VALUE and immediately get this value.
 * EXPECT_OUTCOME_TRUE(val, getResult());
 */
#define EXPECT_OUTCOME_TRUE(val, expr) \
  EXPECT_OUTCOME_TRUE_name(BOOST_OUTCOME_TRY_UNIQUE_NAME, val, expr)

#define _EXPECT_EC(tmp, expr, expected) \
  auto &&tmp = expr;                    \
  EXPECT_TRUE(tmp.has_error());         \
  EXPECT_EQ(tmp.error(), expected);
#define EXPECT_EC(expr, expected) \
  _EXPECT_EC(BOOST_OUTCOME_TRY_UNIQUE_NAME, expr, expected)
