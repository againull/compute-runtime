/*
 * Copyright (C) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "opencl/test/unit_test/command_stream/compute_mode_tests.h"

using namespace NEO;

using ComputeModeRequirementsPvcAndLater = ComputeModeRequirements;

HWTEST2_F(ComputeModeRequirementsPvcAndLater, givenComputeModeCmdSizeWhenLargeGrfModeChangeIsRequiredThenSCMAndPcCommandsSizeIsCalculated, IsXeHpcCore) {
    SetUpImpl<FamilyType>();
    using STATE_COMPUTE_MODE = typename FamilyType::STATE_COMPUTE_MODE;
    using PIPE_CONTROL = typename FamilyType::PIPE_CONTROL;

    auto cmdSize = sizeof(STATE_COMPUTE_MODE) + sizeof(PIPE_CONTROL);

    overrideComputeModeRequest<FamilyType>(false, false, false, false, 128u);
    auto retSize = getCsrHw<FamilyType>()->getCmdSizeForComputeMode();
    EXPECT_EQ(0u, retSize);

    overrideComputeModeRequest<FamilyType>(false, false, false, true, 256u);
    retSize = getCsrHw<FamilyType>()->getCmdSizeForComputeMode();
    EXPECT_EQ(cmdSize, retSize);

    overrideComputeModeRequest<FamilyType>(true, false, false, true, 256u);
    retSize = getCsrHw<FamilyType>()->getCmdSizeForComputeMode();
    EXPECT_EQ(cmdSize, retSize);
}
