/*
 * Copyright (C) 2018-2019 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "runtime/command_stream/command_stream_receiver_hw.h"
#include "runtime/gen_common/aub_mapper.h"
#include "runtime/memory_manager/memory_banks.h"

#include "third_party/aub_stream/headers/hardware_context.h"

namespace aub_stream {
class AubManager;
struct AubStream;
} // namespace aub_stream

namespace OCLRT {
class GraphicsAllocation;
class HardwareContextController;
template <typename GfxFamily>
class CommandStreamReceiverSimulatedCommonHw : public CommandStreamReceiverHw<GfxFamily> {
  protected:
    using CommandStreamReceiverHw<GfxFamily>::CommandStreamReceiverHw;
    using CommandStreamReceiverHw<GfxFamily>::deviceIndex;
    using CommandStreamReceiverHw<GfxFamily>::hwInfo;
    using CommandStreamReceiverHw<GfxFamily>::osContext;
    using AUB = typename AUBFamilyMapper<GfxFamily>::AUB;
    using MiContextDescriptorReg = typename AUB::MiContextDescriptorReg;

    bool getParametersForWriteMemory(GraphicsAllocation &graphicsAllocation, uint64_t &gpuAddress, void *&cpuAddress, size_t &size) const;

  public:
    uint64_t getGTTBits() const {
        return 0u;
    }
    void initGlobalMMIO();
    void initAdditionalMMIO();
    uint64_t getPPGTTAdditionalBits(GraphicsAllocation *gfxAllocation);
    void getGTTData(void *memory, AubGTTData &data);
    uint32_t getMemoryBankForGtt() const;
    static const AubMemDump::LrcaHelper &getCsTraits(EngineInstanceT engineInstance);
    void initEngineMMIO();
    void submitLRCA(const MiContextDescriptorReg &contextDescriptor);
    void setupContext(OsContext &osContext) override;
    virtual void expectMemoryEqual(void *gfxAddress, const void *srcAddress, size_t length);
    virtual void expectMemoryNotEqual(void *gfxAddress, const void *srcAddress, size_t length);
    virtual void pollForCompletion() = 0;
    virtual void pollForCompletionImpl(){};
    virtual bool writeMemory(GraphicsAllocation &gfxAllocation) = 0;
    virtual void writeMemory(uint64_t gpuAddress, void *cpuAddress, size_t size, uint32_t memoryBank, uint64_t entryBits) = 0;
    virtual void writeMemoryWithAubManager(GraphicsAllocation &graphicsAllocation) = 0;

    aub_stream::AubManager *aubManager = nullptr;
    std::unique_ptr<HardwareContextController> hardwareContextController;

    struct EngineInfo {
        void *pLRCA;
        uint32_t ggttLRCA;
        void *pGlobalHWStatusPage;
        uint32_t ggttHWSP;
        void *pRingBuffer;
        uint32_t ggttRingBuffer;
        size_t sizeRingBuffer;
        uint32_t tailRingBuffer;
    } engineInfoTable[EngineType::NUM_ENGINES] = {};

    AubMemDump::AubStream *stream;
};
} // namespace OCLRT
