/*
 * Copyright (C) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/source/command_container/implicit_scaling.h"
#include "shared/source/command_container/implicit_scaling_xehp_and_later.inl"

namespace NEO {

using Family = XE_HPC_COREFamily;

template <>
bool ImplicitScalingDispatch<Family>::pipeControlStallRequired = false;

template struct ImplicitScalingDispatch<Family>;
} // namespace NEO
