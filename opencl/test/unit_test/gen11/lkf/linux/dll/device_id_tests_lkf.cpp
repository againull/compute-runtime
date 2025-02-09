/*
 * Copyright (C) 2019-2021 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/test/common/fixtures/linux/device_id_fixture.h"

using namespace NEO;

TEST_F(DeviceIdTests, GivenLkfSupportedDeviceIdThenHardwareInfoIsCorrect) {
    std::array<DeviceDescriptor, 1> expectedDescriptors = {{
        {0x9840, &LKF_1x8x8::hwInfo, &LKF_1x8x8::setupHardwareInfo, GTTYPE_GT1},
    }};

    testImpl(expectedDescriptors);
}
