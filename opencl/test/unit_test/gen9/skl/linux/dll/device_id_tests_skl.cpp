/*
 * Copyright (C) 2018-2021 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/test/common/fixtures/linux/device_id_fixture.h"

using namespace NEO;

TEST_F(DeviceIdTests, GivenSklSupportedDeviceIdThenHardwareInfoIsCorrect) {
    std::array<DeviceDescriptor, 26> expectedDescriptors = {{
        {0x1902, &SKL_1x2x6::hwInfo, &SKL_1x2x6::setupHardwareInfo, GTTYPE_GT1},
        {0x190B, &SKL_1x2x6::hwInfo, &SKL_1x2x6::setupHardwareInfo, GTTYPE_GT1},
        {0x190A, &SKL_1x2x6::hwInfo, &SKL_1x2x6::setupHardwareInfo, GTTYPE_GT1},
        {0x1906, &SKL_1x2x6::hwInfo, &SKL_1x2x6::setupHardwareInfo, GTTYPE_GT1},
        {0x190E, &SKL_1x2x6::hwInfo, &SKL_1x2x6::setupHardwareInfo, GTTYPE_GT1},

        {0x1917, &SKL_1x3x6::hwInfo, &SKL_1x3x6::setupHardwareInfo, GTTYPE_GT1_5},
        {0x1913, &SKL_1x3x6::hwInfo, &SKL_1x3x6::setupHardwareInfo, GTTYPE_GT1_5},
        {0x1915, &SKL_1x3x6::hwInfo, &SKL_1x3x6::setupHardwareInfo, GTTYPE_GT1_5},

        {0x1912, &SKL_1x3x8::hwInfo, &SKL_1x3x8::setupHardwareInfo, GTTYPE_GT2},
        {0x191B, &SKL_1x3x8::hwInfo, &SKL_1x3x8::setupHardwareInfo, GTTYPE_GT2},
        {0x191A, &SKL_1x3x8::hwInfo, &SKL_1x3x8::setupHardwareInfo, GTTYPE_GT2},
        {0x1916, &SKL_1x3x8::hwInfo, &SKL_1x3x8::setupHardwareInfo, GTTYPE_GT2},
        {0x191E, &SKL_1x3x8::hwInfo, &SKL_1x3x8::setupHardwareInfo, GTTYPE_GT2},
        {0x191D, &SKL_1x3x8::hwInfo, &SKL_1x3x8::setupHardwareInfo, GTTYPE_GT2},
        {0x1921, &SKL_1x3x8::hwInfo, &SKL_1x3x8::setupHardwareInfo, GTTYPE_GT2},
        {0x9905, &SKL_1x3x8::hwInfo, &SKL_1x3x8::setupHardwareInfo, GTTYPE_GT2},

        {0x192B, &SKL_2x3x8::hwInfo, &SKL_2x3x8::setupHardwareInfo, GTTYPE_GT3},
        {0x192D, &SKL_2x3x8::hwInfo, &SKL_2x3x8::setupHardwareInfo, GTTYPE_GT3},
        {0x192A, &SKL_2x3x8::hwInfo, &SKL_2x3x8::setupHardwareInfo, GTTYPE_GT3},
        {0x1923, &SKL_2x3x8::hwInfo, &SKL_2x3x8::setupHardwareInfo, GTTYPE_GT3},
        {0x1926, &SKL_2x3x8::hwInfo, &SKL_2x3x8::setupHardwareInfo, GTTYPE_GT3},
        {0x1927, &SKL_2x3x8::hwInfo, &SKL_2x3x8::setupHardwareInfo, GTTYPE_GT3},

        {0x1932, &SKL_3x3x8::hwInfo, &SKL_3x3x8::setupHardwareInfo, GTTYPE_GT4},
        {0x193B, &SKL_3x3x8::hwInfo, &SKL_3x3x8::setupHardwareInfo, GTTYPE_GT4},
        {0x193A, &SKL_3x3x8::hwInfo, &SKL_3x3x8::setupHardwareInfo, GTTYPE_GT4},
        {0x193D, &SKL_3x3x8::hwInfo, &SKL_3x3x8::setupHardwareInfo, GTTYPE_GT4},
    }};

    testImpl(expectedDescriptors);
}
