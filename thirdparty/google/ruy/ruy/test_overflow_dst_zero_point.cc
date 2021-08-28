/* Copyright 2021 Google LLC. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

// This test covers destination zero_points that cause internal int16 overflow.

// Kernels tend to perform the addition of the destination zero_point in int16.
// Although this happens after the rescaling to the destination scale, it is
// still possible for this int16 addition to overflow. This should be handled
// by saturating, which ensures correct results as the subsequent cast to
// the destination 8-bit type is saturating anyway, so this second saturation
// eats any effect of the previous saturation in the int16 addition of the
// destination zero_point.
// When this is not correctly saturating, a typical effect is wrapping around
// to the opposite end of the range of int16, which causes the latter saturation
// to the int8/uint8 range to saturate to the opposite end of that, resulting
// in a large numerical difference in the output values.

#include <limits>
#include <type_traits>
#include <vector>

#include "ruy/context.h"
#include "ruy/gtest_wrapper.h"
#include "ruy/matrix.h"
#include "ruy/mul_params.h"
#include "ruy/path.h"
#include "ruy/ruy.h"
#include "ruy/test.h"
#include "ruy/tune.h"

namespace ruy {
namespace {

template <typename DstScalar>
void TestOverflowingAdditionOfDestinationZeroPoint(ruy::Context* context,
                                                   int cols,
                                                   DstScalar dst_zero_point) {
  // Set the bias value so that the int16 addition of the zero_point will
  // overflow.
  const int bias_value = dst_zero_point > 0
                             ? std::numeric_limits<std::int16_t>::max()
                             : std::numeric_limits<std::int16_t>::min();
  // This is the end of the DstScalar range that we expect values will be
  // clamped to.
  const int expected_dst_value = dst_zero_point > 0
                                     ? std::numeric_limits<DstScalar>::max()
                                     : std::numeric_limits<DstScalar>::min();

  const std::vector<const std::int8_t> lhs_data(1, 0);
  const std::vector<std::int8_t> rhs_data(cols, 0);
  std::vector<DstScalar> dst_data(cols, 0);

  ruy::MulParams<std::int32_t, DstScalar> mul_params;
  std::int32_t bias_data[1] = {bias_value};
  mul_params.set_bias(bias_data);
  // Set the quantized multiplier to essentially 1 so we get unscaled
  // accumulators in the output, only clamped.
  mul_params.set_multiplier_fixedpoint(
      std::numeric_limits<std::int32_t>::max());

  ruy::Matrix<std::int8_t> lhs;
  ruy::MakeSimpleLayout(1, 1, ruy::Order::kColMajor, lhs.mutable_layout());
  lhs.set_data(lhs_data.data());

  ruy::Matrix<std::int8_t> rhs;
  ruy::MakeSimpleLayout(1, cols, ruy::Order::kColMajor, rhs.mutable_layout());
  rhs.set_data(rhs_data.data());

  ruy::Matrix<DstScalar> dst;
  ruy::MakeSimpleLayout(1, cols, ruy::Order::kColMajor, dst.mutable_layout());
  dst.set_data(dst_data.data());
  dst.set_zero_point(dst_zero_point);

  ruy::Mul(lhs, rhs, mul_params, context, &dst);

  // Check that the DstScalar overflow was clamped, not wrapped around.
  for (auto d : dst_data) {
    EXPECT_EQ(d, expected_dst_value);
  }
}

template <typename DstScalar>
void TestOverflowingAdditionOfDestinationZeroPoint(ruy::Context* context) {
  // Test both a matrix*vector and a general matrix*matrix (in the sense that
  // cols>1) as these may exercise different kernels.
  TestOverflowingAdditionOfDestinationZeroPoint<DstScalar>(context, 1, 1);
  TestOverflowingAdditionOfDestinationZeroPoint<DstScalar>(context, 8, 1);
  if (std::is_signed<DstScalar>::value) {
    TestOverflowingAdditionOfDestinationZeroPoint<DstScalar>(context, 1, -1);
    TestOverflowingAdditionOfDestinationZeroPoint<DstScalar>(context, 8, -1);
  }
}

TEST(RuyTest, OverflowingAdditionOfDestinationZeroPoint) {
  ruy::Context context;
  ruy::Path runtime_enabled_paths = context.get_runtime_enabled_paths();
  for (unsigned bit = 0; bit < 8 * sizeof(ruy::Path); bit++) {
    ruy::Path path = static_cast<ruy::Path>(1 << bit);
    if ((path & runtime_enabled_paths) == ruy::Path::kNone) {
      continue;
    }
    context.set_runtime_enabled_paths(path);
    for (ruy::Tuning tuning :
         {ruy::Tuning::kGeneric, ruy::Tuning::kA55ish, ruy::Tuning::kX1}) {
      fprintf(stderr, "Testing path %s, tuning %s\n", PathName(path),
              TuningName(tuning));
      context.set_explicit_tuning(tuning);
      TestOverflowingAdditionOfDestinationZeroPoint<std::int8_t>(&context);
      TestOverflowingAdditionOfDestinationZeroPoint<std::uint8_t>(&context);
      TestOverflowingAdditionOfDestinationZeroPoint<std::int16_t>(&context);
    }
  }
}

}  // namespace
}  // namespace ruy

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
