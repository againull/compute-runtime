/*
 * Copyright (C) 2021-2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "igfxfmid.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

struct drm_i915_query_item;
namespace NEO {
class Drm;
class IoctlHelper;
enum class CacheRegion : uint16_t;

extern IoctlHelper *ioctlHelperFactory[IGFX_MAX_PRODUCT];

struct MemoryClassInstance {
    uint16_t memoryClass;
    uint16_t memoryInstance;
};

struct EngineClassInstance {
    uint16_t engineClass;
    uint16_t engineInstance;
};

struct MemoryRegion {
    MemoryClassInstance region;
    uint64_t probedSize;
    uint64_t unallocatedSize;
};

struct EngineCapabilities {
    EngineClassInstance engine;
    uint64_t capabilities;
};

struct DistanceInfo {
    MemoryClassInstance region;
    EngineClassInstance engine;
    int32_t distance;
};

class IoctlHelper {
  public:
    virtual ~IoctlHelper() {}
    static IoctlHelper *get(Drm *drm);
    static uint32_t ioctl(Drm *drm, unsigned long request, void *arg);

    virtual uint32_t createGemExt(Drm *drm, const std::vector<MemoryClassInstance> &memClassInstances, size_t allocSize, uint32_t &handle) = 0;
    virtual std::vector<MemoryRegion> translateToMemoryRegions(const std::vector<uint8_t> &regionInfo) = 0;
    virtual CacheRegion closAlloc(Drm *drm) = 0;
    virtual uint16_t closAllocWays(Drm *drm, CacheRegion closIndex, uint16_t cacheLevel, uint16_t numWays) = 0;
    virtual CacheRegion closFree(Drm *drm, CacheRegion closIndex) = 0;
    virtual int waitUserFence(Drm *drm, uint32_t ctxId, uint64_t address,
                              uint64_t value, uint32_t dataWidth, int64_t timeout, uint16_t flags) = 0;
    virtual uint32_t getHwConfigIoctlVal() = 0;
    virtual uint32_t getAtomicAdvise(bool isNonAtomic) = 0;
    virtual uint32_t getPreferredLocationAdvise() = 0;
    virtual bool setVmBoAdvise(Drm *drm, int32_t handle, uint32_t attribute, void *region) = 0;
    virtual uint32_t getDirectSubmissionFlag() = 0;
    virtual int32_t getMemRegionsIoctlVal() = 0;
    virtual int32_t getEngineInfoIoctlVal() = 0;
    virtual std::vector<EngineCapabilities> translateToEngineCaps(const std::vector<uint8_t> &data) = 0;
    virtual uint32_t queryDistances(Drm *drm, std::vector<drm_i915_query_item> &queryItems, std::vector<DistanceInfo> &distanceInfos) = 0;
    virtual int32_t getComputeEngineClass() = 0;
};

class IoctlHelperUpstream : public IoctlHelper {
  public:
    uint32_t createGemExt(Drm *drm, const std::vector<MemoryClassInstance> &memClassInstances, size_t allocSize, uint32_t &handle) override;
    std::vector<MemoryRegion> translateToMemoryRegions(const std::vector<uint8_t> &regionInfo) override;
    CacheRegion closAlloc(Drm *drm) override;
    uint16_t closAllocWays(Drm *drm, CacheRegion closIndex, uint16_t cacheLevel, uint16_t numWays) override;
    CacheRegion closFree(Drm *drm, CacheRegion closIndex) override;
    int waitUserFence(Drm *drm, uint32_t ctxId, uint64_t address,
                      uint64_t value, uint32_t dataWidth, int64_t timeout, uint16_t flags) override;
    uint32_t getHwConfigIoctlVal() override;
    uint32_t getAtomicAdvise(bool isNonAtomic) override;
    uint32_t getPreferredLocationAdvise() override;
    bool setVmBoAdvise(Drm *drm, int32_t handle, uint32_t attribute, void *region) override;
    uint32_t getDirectSubmissionFlag() override;
    int32_t getMemRegionsIoctlVal() override;
    int32_t getEngineInfoIoctlVal() override;
    std::vector<EngineCapabilities> translateToEngineCaps(const std::vector<uint8_t> &data) override;
    uint32_t queryDistances(Drm *drm, std::vector<drm_i915_query_item> &queryItems, std::vector<DistanceInfo> &distanceInfos) override;
    int32_t getComputeEngineClass() override;
};

template <PRODUCT_FAMILY gfxProduct>
class IoctlHelperImpl : public IoctlHelperUpstream {
  public:
    static IoctlHelper *get() {
        static IoctlHelperImpl<gfxProduct> instance;
        return &instance;
    }
    uint32_t createGemExt(Drm *drm, const std::vector<MemoryClassInstance> &memClassInstances, size_t allocSize, uint32_t &handle) override;
    std::vector<MemoryRegion> translateToMemoryRegions(const std::vector<uint8_t> &regionInfo) override;
};

class IoctlHelperPrelim20 : public IoctlHelper {
  public:
    uint32_t createGemExt(Drm *drm, const std::vector<MemoryClassInstance> &memClassInstances, size_t allocSize, uint32_t &handle) override;
    std::vector<MemoryRegion> translateToMemoryRegions(const std::vector<uint8_t> &regionInfo) override;
    CacheRegion closAlloc(Drm *drm) override;
    uint16_t closAllocWays(Drm *drm, CacheRegion closIndex, uint16_t cacheLevel, uint16_t numWays) override;
    CacheRegion closFree(Drm *drm, CacheRegion closIndex) override;
    int waitUserFence(Drm *drm, uint32_t ctxId, uint64_t address,
                      uint64_t value, uint32_t dataWidth, int64_t timeout, uint16_t flags) override;
    uint32_t getHwConfigIoctlVal() override;
    uint32_t getAtomicAdvise(bool isNonAtomic) override;
    uint32_t getPreferredLocationAdvise() override;
    bool setVmBoAdvise(Drm *drm, int32_t handle, uint32_t attribute, void *region) override;
    uint32_t getDirectSubmissionFlag() override;
    int32_t getMemRegionsIoctlVal() override;
    int32_t getEngineInfoIoctlVal() override;
    std::vector<EngineCapabilities> translateToEngineCaps(const std::vector<uint8_t> &data) override;
    uint32_t queryDistances(Drm *drm, std::vector<drm_i915_query_item> &queryItems, std::vector<DistanceInfo> &distanceInfos) override;
    int32_t getComputeEngineClass() override;
};

} // namespace NEO
