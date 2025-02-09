/*
 * Copyright (C) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/source/xe_hpg_core/hw_info.h"
#include "shared/test/common/helpers/unit_test_helper.h"
#include "shared/test/common/helpers/unit_test_helper.inl"
using Family = NEO::XE_HPG_COREFamily;
#include "shared/test/common/helpers/unit_test_helper_xehp_and_later.inl"

namespace NEO {
template <>
const AuxTranslationMode UnitTestHelper<Family>::requiredAuxTranslationMode = AuxTranslationMode::Blit;

template <>
const bool UnitTestHelper<Family>::additionalMiFlushDwRequired = true;

template <>
uint32_t UnitTestHelper<Family>::getDebugModeRegisterOffset() {
    return 0x20d8;
}

template <>
uint32_t UnitTestHelper<Family>::getDebugModeRegisterValue() {
    return (1u << 5) | (1u << 21);
}

template <>
uint32_t UnitTestHelper<Family>::getTdCtlRegisterOffset() {
    return 0xe400;
}

template <>
uint32_t UnitTestHelper<Family>::getTdCtlRegisterValue() {
    return (1u << 7) | (1u << 4) | (1u << 2) | (1u << 0);
}

template struct UnitTestHelper<Family>;
} // namespace NEO
