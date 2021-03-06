/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>

#include <arrayfire.h>

namespace fl {

/**
 * Returns true if two arrays are of same type and are element-wise equal within
 * given tolerance limit.
 *
 * @param [a,b] input arrays to compare
 * @param absTolerance absolute tolerance allowed
 *
 */
bool allClose(
    const af::array& a,
    const af::array& b,
    double absTolerance = 1e-5);

// Returns high resolution time formatted as:
// MMDD HH MM SS UUUUUU
// 0206 08:42:42.123456
std::string dateTimeWithMicroSeconds();

// Returns round-up result of integer division.
// throws invalid_argument exception on zero denominator.
size_t divRoundUp(size_t numerator, size_t denominator);

} // namespace fl
