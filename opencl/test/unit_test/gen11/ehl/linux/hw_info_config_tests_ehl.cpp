/*
 * Copyright (C) 2019-2021 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "opencl/test/unit_test/helpers/gtest_helpers.h"
#include "opencl/test/unit_test/os_interface/linux/hw_info_config_linux_tests.h"

using namespace NEO;

struct HwInfoConfigTestLinuxEhl : HwInfoConfigTestLinux {
    void SetUp() override {
        HwInfoConfigTestLinux::SetUp();

        drm->storedDeviceID = IEHL_1x4x8_SUPERSKU_DEVICE_A0_ID;
        drm->setGtType(GTTYPE_GT1);
        drm->storedSSVal = 8;
    }
};

EHLTEST_F(HwInfoConfigTestLinuxEhl, GivenEhlThenHwInfoIsCorrect) {
    auto hwInfoConfig = HwInfoConfigHw<IGFX_ELKHARTLAKE>::get();
    int ret = hwInfoConfig->configureHwInfoDrm(&pInHwInfo, &outHwInfo, osInterface);
    EXPECT_EQ(0, ret);
    EXPECT_EQ((unsigned short)drm->storedDeviceID, outHwInfo.platform.usDeviceID);
    EXPECT_EQ((unsigned short)drm->storedDeviceRevID, outHwInfo.platform.usRevId);
    EXPECT_EQ((uint32_t)drm->storedEUVal, outHwInfo.gtSystemInfo.EUCount);
    EXPECT_EQ((uint32_t)drm->storedSSVal, outHwInfo.gtSystemInfo.SubSliceCount);
    EXPECT_EQ(1u, outHwInfo.gtSystemInfo.SliceCount);

    EXPECT_EQ(GTTYPE_GT1, outHwInfo.platform.eGTType);
    EXPECT_TRUE(outHwInfo.featureTable.flags.ftrGT1);
    EXPECT_FALSE(outHwInfo.featureTable.flags.ftrGT1_5);
    EXPECT_FALSE(outHwInfo.featureTable.flags.ftrGT2);
    EXPECT_FALSE(outHwInfo.featureTable.flags.ftrGT3);
    EXPECT_FALSE(outHwInfo.featureTable.flags.ftrGT4);
    EXPECT_FALSE(outHwInfo.featureTable.flags.ftrGTA);
    EXPECT_FALSE(outHwInfo.featureTable.flags.ftrGTC);
    EXPECT_FALSE(outHwInfo.featureTable.flags.ftrGTX);
    EXPECT_FALSE(outHwInfo.featureTable.flags.ftrTileY);
}

EHLTEST_F(HwInfoConfigTestLinuxEhl, GivenInvalidDeviceIdWhenConfiguringHwInfoThenNegativeOneReturned) {
    auto hwInfoConfig = HwInfoConfigHw<IGFX_ELKHARTLAKE>::get();

    drm->storedRetValForDeviceID = -1;
    int ret = hwInfoConfig->configureHwInfoDrm(&pInHwInfo, &outHwInfo, osInterface);
    EXPECT_EQ(-1, ret);

    drm->storedRetValForDeviceID = 0;
    drm->storedRetValForDeviceRevID = -1;
    ret = hwInfoConfig->configureHwInfoDrm(&pInHwInfo, &outHwInfo, osInterface);
    EXPECT_EQ(-1, ret);

    drm->storedRetValForDeviceRevID = 0;
    drm->failRetTopology = true;
    drm->storedRetValForEUVal = -1;
    ret = hwInfoConfig->configureHwInfoDrm(&pInHwInfo, &outHwInfo, osInterface);
    EXPECT_EQ(-1, ret);

    drm->storedRetValForEUVal = 0;
    drm->storedRetValForSSVal = -1;
    ret = hwInfoConfig->configureHwInfoDrm(&pInHwInfo, &outHwInfo, osInterface);
    EXPECT_EQ(-1, ret);
}

template <typename T>
class EhlHwInfoTests : public ::testing::Test {};
TEST(EhlHwInfoTests, WhenGtIsSetupThenGtSystemInfoIsCorrect) {
    HardwareInfo hwInfo = *defaultHwInfo;
    auto executionEnvironment = std::make_unique<ExecutionEnvironment>();
    executionEnvironment->prepareRootDeviceEnvironments(1);
    executionEnvironment->rootDeviceEnvironments[0]->setHwInfo(defaultHwInfo.get());
    DrmMock drm(*executionEnvironment->rootDeviceEnvironments[0]);
    GT_SYSTEM_INFO &gtSystemInfo = hwInfo.gtSystemInfo;
    DeviceDescriptor device = {0, &hwInfo, &EHL_HW_CONFIG::setupHardwareInfo, GTTYPE_GT1};

    int ret = drm.setupHardwareInfo(&device, false);

    EXPECT_EQ(ret, 0);
    EXPECT_GT(gtSystemInfo.EUCount, 0u);
    EXPECT_GT(gtSystemInfo.ThreadCount, 0u);
    EXPECT_GT(gtSystemInfo.SliceCount, 0u);
    EXPECT_GT(gtSystemInfo.SubSliceCount, 0u);
    EXPECT_GT(gtSystemInfo.DualSubSliceCount, 0u);
    EXPECT_GT_VAL(gtSystemInfo.L3CacheSizeInKb, 0u);
    EXPECT_EQ(gtSystemInfo.CsrSizeInMb, 8u);
    EXPECT_FALSE(gtSystemInfo.IsDynamicallyPopulated);
}
