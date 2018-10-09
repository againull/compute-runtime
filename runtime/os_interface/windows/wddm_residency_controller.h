/*
 * Copyright (C) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "runtime/memory_manager/residency_container.h"

#include <atomic>

namespace OCLRT {

class GraphicsAllocation;
class WddmAllocation;

class WddmResidencyController {
  public:
    WddmResidencyController(uint32_t osContextId);

    void acquireLock();
    void releaseLock();

    void acquireTrimCallbackLock();
    void releaseTrimCallbackLock();

    WddmAllocation *getTrimCandidateHead();
    void addToTrimCandidateList(GraphicsAllocation *allocation);
    void removeFromTrimCandidateList(GraphicsAllocation *allocation, bool compactList);
    void removeFromTrimCandidateListIfUsed(WddmAllocation *allocation, bool compactList);
    void checkTrimCandidateCount();

    bool checkTrimCandidateListCompaction();
    void compactTrimCandidateList();

    uint64_t getLastTrimFenceValue() { return lastTrimFenceValue; }
    void setLastTrimFenceValue(uint64_t value) { lastTrimFenceValue = value; }
    const ResidencyContainer &peekTrimCandidateList() const { return trimCandidateList; }
    uint32_t peekTrimCandidatesCount() const { return trimCandidatesCount; }

  protected:
    uint32_t osContextId;
    std::atomic<bool> lock = false;
    std::atomic_flag trimCallbackLock = ATOMIC_FLAG_INIT;
    uint64_t lastTrimFenceValue = 0u;
    ResidencyContainer trimCandidateList;
    uint32_t trimCandidatesCount = 0;
};
} // namespace OCLRT
