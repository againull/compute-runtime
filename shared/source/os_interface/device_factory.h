/*
 * Copyright (C) 2018-2021 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

namespace NEO {

class ExecutionEnvironment;
class Device;
bool prepareDeviceEnvironments(ExecutionEnvironment &executionEnvironment);
bool prepareDeviceEnvironment(ExecutionEnvironment &executionEnvironment, std::string &osPciPath, const uint32_t rootDeviceIndex);
class DeviceFactory {
  public:
    static bool prepareDeviceEnvironments(ExecutionEnvironment &executionEnvironment);
    static bool prepareDeviceEnvironment(ExecutionEnvironment &executionEnvironment, std::string &osPciPath, const uint32_t rootDeviceIndex);
    static bool prepareDeviceEnvironmentsForProductFamilyOverride(ExecutionEnvironment &executionEnvironment);
    static std::vector<std::unique_ptr<Device>> createDevices(ExecutionEnvironment &executionEnvironment);
    static std::unique_ptr<Device> createDevice(ExecutionEnvironment &executionEnvironment, std::string &osPciPath, const uint32_t rootDeviceIndex);
    static bool isHwModeSelected();

    static std::unique_ptr<Device> (*createRootDeviceFunc)(ExecutionEnvironment &executionEnvironment, uint32_t rootDeviceIndex);
    static bool (*createMemoryManagerFunc)(ExecutionEnvironment &executionEnvironment);
};
} // namespace NEO
