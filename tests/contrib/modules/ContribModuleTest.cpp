/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include "flashlight/autograd/autograd.h"
#include "flashlight/common/common.h"
#include "flashlight/contrib/modules/modules.h"
#include "flashlight/nn/nn.h"

using namespace fl;

TEST(ModuleTest, ResidualFwd) {
  auto conv = Conv2D(30, 50, 9, 7, 2, 3, 3, 2);
  auto bn = BatchNorm(2, 50);
  auto relu = ReLU();

  int batchsize = 10;
  auto input = Variable(af::randu(120, 100, 30, batchsize), false);

  auto outputConv = conv.forward(input);
  auto outputBn = bn.forward(outputConv);

  auto resModule1 = Residual();
  resModule1.add(conv);
  resModule1.add(bn);
  resModule1.add(relu);
  resModule1.addShortcut(1, 3);

  auto output1 = resModule1.forward(input);
  auto output1True = relu.forward(outputBn + outputConv);
  ASSERT_TRUE(allClose(output1, output1True));

  auto resModule2 = Residual();
  resModule2.add(conv);
  resModule2.add(bn);
  resModule2.add(relu);
  resModule2.addShortcut(1, 4);
  resModule2.addShortcut(1, 3);
  resModule2.addShortcut(2, 4);

  auto output2 = resModule2.forward(input);
  auto output2True =
      relu.forward(outputBn + outputConv) + outputBn + outputConv;
  ASSERT_TRUE(allClose(output2, output2True));
}

TEST(ModuleTest, ResidualFwdWithProjection) {
  const float proj1FwdScale = 0.24;
  const float proj2FwdScale = 0.5;
  const float linFwdScale = 0.3;

  auto linear1 = Linear(12, 8);
  auto relu1 = ReLU();
  auto linear2 = Linear(8, 4);
  auto relu2 = ReLU();
  auto linear3 = Linear(4, 4);
  auto relu3 = ReLU();
  auto projection1 = Linear(8, 4);
  auto projection2 = Linear(12, 4);

  auto input = Variable(af::randu(12, 10, 3, 4), false);
  auto output1True = linear1.forward(input);
  auto outputTrue = relu1.forward(output1True);
  outputTrue = linear2.forward(outputTrue * linFwdScale);
  outputTrue = relu2.forward(
      (outputTrue + projection1.forward(output1True)) * proj1FwdScale);
  outputTrue = (outputTrue + projection2.forward(input)) * proj2FwdScale;
  outputTrue = linear3.forward(outputTrue);
  outputTrue = relu3.forward(outputTrue) + outputTrue;

  auto resModule = Residual();
  resModule.add(linear1);
  resModule.add(relu1);
  resModule.add(linear2);
  resModule.addScale(3, linFwdScale);
  resModule.add(relu2);
  resModule.addShortcut(1, 4, projection1);
  resModule.addScale(4, proj1FwdScale);
  resModule.add(linear3);
  resModule.addShortcut(0, 5, projection2);
  resModule.addScale(5, proj2FwdScale);
  resModule.add(relu3);
  resModule.addShortcut(5, 7);

  auto outputRes = resModule.forward(input);
  ASSERT_TRUE(allClose(outputRes, outputTrue));
}

TEST(ModuleTest, AsymmetricConv1DFwd) {
  int batchsize = 10;
  int timesteps = 120;
  int c = 32;

  auto conv = AsymmetricConv1D(c, c, 5, 1, -1, 0, 1); // use only past
  auto input = Variable(af::randu(timesteps, 1, c, batchsize), false);

  auto output = conv.forward(input);

  ASSERT_EQ(output.dims(0), timesteps);
  ASSERT_EQ(output.dims(1), 1);
  ASSERT_EQ(output.dims(2), c);

  auto convFuture = AsymmetricConv1D(c, c, 5, 1, -1, 1, 1); // use only future
  auto outputFuture = convFuture.forward(input);
  ASSERT_EQ(outputFuture.dims(0), timesteps);
  ASSERT_EQ(outputFuture.dims(1), 1);
  ASSERT_EQ(outputFuture.dims(2), c);

  ASSERT_FALSE(allClose(output, outputFuture));
}

TEST(ModuleTest, TransformerFwd) {
  int batchsize = 10;
  int timesteps = 120;
  int c = 32;
  int nheads = 4;

  auto tr =
      Transformer(c, c / nheads, c, nheads, timesteps, 0.2, 0.1, false, false);
  auto input = Variable(af::randu(c, timesteps, batchsize, 1), false);

  auto output = tr.forward({input});

  ASSERT_EQ(output[0].dims(0), c);
  ASSERT_EQ(output[0].dims(1), timesteps);
  ASSERT_EQ(output[0].dims(2), batchsize);
}

TEST(ModuleTest, PositionEmbeddingFwd) {
  int batchsize = 10;
  int timesteps = 120;
  int csz = 256;

  auto posemb = PositionEmbedding(csz, timesteps, 0.5);
  auto input = Variable(af::randu(csz, timesteps, batchsize, 1), false);

  auto output = posemb.forward({input});

  ASSERT_EQ(output[0].dims(0), csz);
  ASSERT_EQ(output[0].dims(1), timesteps);
  ASSERT_EQ(output[0].dims(2), batchsize);

  ASSERT_FALSE(allClose(output[0], input));
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
