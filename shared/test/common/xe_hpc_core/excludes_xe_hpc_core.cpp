/*
 * Copyright (C) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/test/common/test_macros/test.h"

HWTEST_EXCLUDE_PRODUCT(CommandEncodeStatesTest, givenSlmTotalSizeEqualZeroWhenDispatchingKernelThenSharedMemorySizeIsSetCorrectly, IGFX_XE_HPC_CORE);
HWTEST_EXCLUDE_PRODUCT(CommandEncodeStatesTest, givenOverrideSlmTotalSizeDebugVariableWhenDispatchingKernelThenSharedMemorySizeIsSetCorrectly, IGFX_XE_HPC_CORE);
HWTEST_EXCLUDE_PRODUCT(WalkerPartitionTests, givenMiAtomicWhenItIsProgrammedThenAllFieldsAreSetCorrectly, IGFX_XE_HPC_CORE);
HWTEST_EXCLUDE_PRODUCT(WalkerPartitionTests, givenProgramBatchBufferStartCommandWhenItIsCalledThenCommandIsProgrammedCorrectly, IGFX_XE_HPC_CORE);
HWTEST_EXCLUDE_PRODUCT(XeHPAndLaterPreemptionTests, GivenDebuggerUsedWhenProgrammingStateSipThenStateSipIsAdded, IGFX_XE_HPC_CORE);
