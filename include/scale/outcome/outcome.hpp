/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SCALE_OUTCOME_OUTCOME
#define SCALE_OUTCOME_OUTCOME

#pragma warning(push)
#pragma warning(disable:4583)
#pragma warning(disable:4582)
#include <boost/outcome/outcome.hpp>
#include <boost/outcome/try.hpp>
#pragma warning(pop)

#include "outcome-register.hpp"

namespace scale::outcome {

using namespace BOOST_OUTCOME_V2_NAMESPACE;

template <class R, class S = std::error_code,
          class NoValuePolicy = policy::default_policy<R, S, void>>
using result = basic_result<R, S, NoValuePolicy>;

} // namespace scale::outcome

// To define OUTCOME_TRY macro, we will need to create OUTCOME_TRY_1 and
// OUTCOME_TRY_2 depending on number of arguments
#define OUTCOME_TRY_1(...) BOOST_OUTCOME_TRY(__VA_ARGS__)
#define OUTCOME_TRY_2(...) BOOST_OUTCOME_TRY(auto __VA_ARGS__)

// trick from https://stackoverflow.com/a/11763277 to overload OUTCOME_TRY
#define GET_MACRO(_1, _2, NAME, ...) NAME
#define OUTCOME_TRY(...) \
  GET_MACRO(__VA_ARGS__, OUTCOME_TRY_2, OUTCOME_TRY_1)(__VA_ARGS__)

#endif // SCALE_OUTCOME_OUTCOME
