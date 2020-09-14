/*
 * Copyright (C) 2019-2020 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "shared/source/device/sub_device.h"
#include "shared/source/memory_manager/graphics_allocation.h"

namespace NEO {
struct ImageInfo;
struct AllocationProperties {
    union {
        struct {
            uint32_t allocateMemory : 1;
            uint32_t flushL3RequiredForRead : 1;
            uint32_t flushL3RequiredForWrite : 1;
            uint32_t forcePin : 1;
            uint32_t uncacheable : 1;
            uint32_t multiOsContextCapable : 1;
            uint32_t readOnlyMultiStorage : 1;
            uint32_t shareable : 1;
            uint32_t resource48Bit : 1;
            uint32_t isUSMHostAllocation : 1;
            uint32_t isUSMDeviceAllocation : 1;
            uint32_t use32BitExtraPool : 1;
            uint32_t reserved : 20;
        } flags;
        uint32_t allFlags = 0;
    };
    static_assert(sizeof(AllocationProperties::flags) == sizeof(AllocationProperties::allFlags), "");
    uint32_t rootDeviceIndex = std::numeric_limits<uint32_t>::max();
    size_t size = 0;
    size_t alignment = 0;
    GraphicsAllocation::AllocationType allocationType = GraphicsAllocation::AllocationType::UNKNOWN;
    ImageInfo *imgInfo = nullptr;
    bool multiStorageResource = false;
    DeviceBitfield subDevicesBitfield{};
    uint64_t gpuAddress = 0;
    OsContext *osContext = nullptr;

    AllocationProperties(uint32_t rootDeviceIndex, size_t size,
                         GraphicsAllocation::AllocationType allocationType, DeviceBitfield subDevicesBitfieldParam)
        : AllocationProperties(rootDeviceIndex, true, size, allocationType, false, subDevicesBitfieldParam) {}

    AllocationProperties(uint32_t rootDeviceIndex, bool allocateMemory,
                         ImageInfo &imgInfo,
                         GraphicsAllocation::AllocationType allocationType,
                         DeviceBitfield subDevicesBitfieldParam)
        : AllocationProperties(rootDeviceIndex, allocateMemory, 0u, allocationType, false, subDevicesBitfieldParam) {
        this->imgInfo = &imgInfo;
    }

    AllocationProperties(uint32_t rootDeviceIndex,
                         bool allocateMemory,
                         size_t size,
                         GraphicsAllocation::AllocationType allocationType,
                         bool isMultiStorageAllocation,
                         DeviceBitfield subDevicesBitfieldParam)
        : AllocationProperties(rootDeviceIndex, allocateMemory, size, allocationType, false, isMultiStorageAllocation, subDevicesBitfieldParam) {}

    AllocationProperties(uint32_t rootDeviceIndexParam,
                         bool allocateMemoryParam,
                         size_t sizeParam,
                         GraphicsAllocation::AllocationType allocationTypeParam,
                         bool multiOsContextCapableParam,
                         bool isMultiStorageAllocationParam,
                         DeviceBitfield subDevicesBitfieldParam)
        : rootDeviceIndex(rootDeviceIndexParam),
          size(sizeParam),
          allocationType(allocationTypeParam),
          multiStorageResource(isMultiStorageAllocationParam),
          subDevicesBitfield(subDevicesBitfieldParam) {
        allFlags = 0;
        flags.flushL3RequiredForRead = 1;
        flags.flushL3RequiredForWrite = 1;
        flags.allocateMemory = allocateMemoryParam;
        flags.multiOsContextCapable = multiOsContextCapableParam;
    }
};

struct AllocationData {
    union {
        struct {
            uint32_t allocateMemory : 1;
            uint32_t allow64kbPages : 1;
            uint32_t allow32Bit : 1;
            uint32_t useSystemMemory : 1;
            uint32_t forcePin : 1;
            uint32_t uncacheable : 1;
            uint32_t flushL3 : 1;
            uint32_t preferRenderCompressed : 1;
            uint32_t multiOsContextCapable : 1;
            uint32_t requiresCpuAccess : 1;
            uint32_t shareable : 1;
            uint32_t resource48Bit : 1;
            uint32_t isUSMHostAllocation : 1;
            uint32_t use32BitExtraPool : 1;
            uint32_t reserved : 18;
        } flags;
        uint32_t allFlags = 0;
    };
    static_assert(sizeof(AllocationData::flags) == sizeof(AllocationData::allFlags), "");
    GraphicsAllocation::AllocationType type = GraphicsAllocation::AllocationType::UNKNOWN;
    const void *hostPtr = nullptr;
    uint64_t gpuAddress = 0;
    size_t size = 0;
    size_t alignment = 0;
    StorageInfo storageInfo = {};
    ImageInfo *imgInfo = nullptr;
    uint32_t rootDeviceIndex = 0;
    OsContext *osContext = nullptr;
};
} // namespace NEO
