/*
 * Copyright (C) 2018-2021 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/source/utilities/tag_allocator.h"
#include "shared/test/common/cmd_parse/hw_parse.h"
#include "shared/test/common/helpers/debug_manager_state_restore.h"
#include "shared/test/common/helpers/unit_test_helper.h"
#include "shared/test/common/mocks/mock_device.h"

#include "opencl/source/command_queue/gpgpu_walker.h"
#include "opencl/source/helpers/hardware_commands_helper.h"
#include "opencl/test/unit_test/fixtures/device_host_queue_fixture.h"
#include "opencl/test/unit_test/fixtures/device_queue_matcher.h"
#include "opencl/test/unit_test/fixtures/execution_model_fixture.h"
#include "opencl/test/unit_test/mocks/mock_context.h"
#include "opencl/test/unit_test/mocks/mock_device_queue.h"
#include "opencl/test/unit_test/mocks/mock_kernel.h"

#include "hw_cmds.h"

#include <memory>

using namespace NEO;
using namespace DeviceHostQueue;

HWTEST2_F(DeviceQueueHwTest, WhenResettingDeviceQueueThenQueueMatchesUnderlyingBuffer, DeviceEnqueueSupport) {
    // profiling disabled
    deviceQueue = createQueueObject();
    ASSERT_NE(deviceQueue, nullptr);
    auto deviceQueueHw = castToHwType<FamilyType>(deviceQueue);

    auto expected = getExpectedgilCmdQueueAfterReset(deviceQueue);
    deviceQueueHw->resetDeviceQueue();

    EXPECT_EQ(0, memcmp(deviceQueueHw->getQueueBuffer()->getUnderlyingBuffer(),
                        &expected, sizeof(IGIL_CommandQueue)));

    delete deviceQueue;

    //profiling enabled
    deviceQueue = createQueueObject(deviceQueueProperties::minimumPropertiesWithProfiling);
    ASSERT_NE(deviceQueue, nullptr);
    deviceQueueHw = castToHwType<FamilyType>(deviceQueue);

    expected = getExpectedgilCmdQueueAfterReset(deviceQueue);
    deviceQueueHw->resetDeviceQueue();

    EXPECT_EQ(1u, expected.m_controls.m_IsProfilingEnabled);

    EXPECT_EQ(0, memcmp(deviceQueue->getQueueBuffer()->getUnderlyingBuffer(),
                        &expected, sizeof(IGIL_CommandQueue)));
    delete deviceQueue;
}

HWTEST2_F(DeviceQueueHwTest, WhenResettingDeviceQueueThenFirstStackElementAtValueOneDeviceEnqueueSupport, DeviceEnqueueSupport) {
    deviceQueue = createQueueObject();
    ASSERT_NE(deviceQueue, nullptr);
    auto deviceQueueHw = castToHwType<FamilyType>(deviceQueue);

    deviceQueueHw->resetDeviceQueue();

    auto stack = static_cast<uint32_t *>(deviceQueue->getStackBuffer()->getUnderlyingBuffer());
    stack += ((deviceQueue->getStackBuffer()->getUnderlyingBufferSize() / sizeof(uint32_t)) - 1);
    EXPECT_EQ(*stack, 1u); // first stack element in surface at value "1"
    delete deviceQueue;
}

HWTEST2_F(DeviceQueueHwTest, GivenNullHardwareIsEnabledWhenAcquiringEmCrticalSectionThenSectionIsNotAcquired, DeviceEnqueueSupport) {
    DebugManagerStateRestore dbgRestorer;

    DebugManager.flags.EnableNullHardware.set(1);

    deviceQueue = createQueueObject();
    ASSERT_NE(deviceQueue, nullptr);
    auto deviceQueueHw = castToHwType<FamilyType>(deviceQueue);

    deviceQueueHw->acquireEMCriticalSection();

    EXPECT_TRUE(deviceQueueHw->isEMCriticalSectionFree());
    delete deviceQueue;
}

HWTEST2_F(DeviceQueueHwTest, WhenGettinCsPrefetchSizeThenSizeIsGreaterThanZero, DeviceEnqueueSupport) {
    auto mockDeviceQueueHw = new MockDeviceQueueHw<FamilyType>(pContext, device,
                                                               deviceQueueProperties::minimumProperties[0]);

    EXPECT_NE(0u, mockDeviceQueueHw->getCSPrefetchSize());
    delete mockDeviceQueueHw;
}

HWTEST2_F(DeviceQueueHwTest, GivenAddLriCmdWithArbCheckWhenGettingSlbCsThenParamsAreCorrect, DeviceEnqueueSupport) {
    using MI_LOAD_REGISTER_IMM = typename FamilyType::MI_LOAD_REGISTER_IMM;
    auto mockDeviceQueueHw = new MockDeviceQueueHw<FamilyType>(pContext, device,
                                                               deviceQueueProperties::minimumProperties[0]);

    mockDeviceQueueHw->addLriCmd(true);

    HardwareParse hwParser;
    auto *slbCS = mockDeviceQueueHw->getSlbCS();

    hwParser.parseCommands<FamilyType>(*slbCS, 0);
    auto loadRegImmItor = find<MI_LOAD_REGISTER_IMM *>(hwParser.cmdList.begin(), hwParser.cmdList.end());

    ASSERT_NE(hwParser.cmdList.end(), loadRegImmItor);

    MI_LOAD_REGISTER_IMM *loadRegImm = (MI_LOAD_REGISTER_IMM *)*loadRegImmItor;

    EXPECT_EQ(0x2248u, loadRegImm->getRegisterOffset());
    EXPECT_EQ(0x100u, loadRegImm->getDataDword());

    EXPECT_EQ(sizeof(MI_LOAD_REGISTER_IMM), slbCS->getUsed());
    delete mockDeviceQueueHw;
}

HWTEST2_F(DeviceQueueHwTest, GivenAddLriCmdWithoutArbCheckWhenGettingSlbCsThenParamsAreCorrect, DeviceEnqueueSupport) {
    using MI_LOAD_REGISTER_IMM = typename FamilyType::MI_LOAD_REGISTER_IMM;
    auto mockDeviceQueueHw = new MockDeviceQueueHw<FamilyType>(pContext, device,
                                                               deviceQueueProperties::minimumProperties[0]);

    mockDeviceQueueHw->addLriCmd(false);

    HardwareParse hwParser;
    auto *slbCS = mockDeviceQueueHw->getSlbCS();

    hwParser.parseCommands<FamilyType>(*slbCS, 0);
    auto loadRegImmItor = find<MI_LOAD_REGISTER_IMM *>(hwParser.cmdList.begin(), hwParser.cmdList.end());

    ASSERT_NE(hwParser.cmdList.end(), loadRegImmItor);

    MI_LOAD_REGISTER_IMM *loadRegImm = (MI_LOAD_REGISTER_IMM *)*loadRegImmItor;

    EXPECT_EQ(0x2248u, loadRegImm->getRegisterOffset());
    EXPECT_EQ(0u, loadRegImm->getDataDword());

    EXPECT_EQ(sizeof(MI_LOAD_REGISTER_IMM), slbCS->getUsed());
    delete mockDeviceQueueHw;
}

HWTEST2_F(DeviceQueueHwTest, GivenDeviceQueueHWWhenEventPoolIsCreatedThenTimestampResolutionIsSet, DeviceEnqueueSupport) {
    auto timestampResolution = static_cast<float>(device->getProfilingTimerResolution());

    auto deviceQueue = std::unique_ptr<DeviceQueue>(createQueueObject());
    ASSERT_NE(deviceQueue, nullptr);

    auto eventPoolBuffer = reinterpret_cast<IGIL_EventPool *>(deviceQueue->getEventPoolBuffer()->getUnderlyingBuffer());

    EXPECT_FLOAT_EQ(timestampResolution, eventPoolBuffer->m_TimestampResolution);
}

class DeviceQueueSlb : public DeviceQueueHwTest {
  public:
    template <typename Cmd>
    void *compareCmds(void *position, Cmd &cmd) {
        EXPECT_EQ(0, memcmp(position, &cmd, sizeof(Cmd)));
        return ptrOffset(position, sizeof(Cmd));
    }
    void *compareCmdsWithSize(void *position, void *cmd, size_t size) {
        EXPECT_EQ(0, memcmp(position, cmd, size));
        return ptrOffset(position, size);
    }
};

HWTEST2_F(DeviceQueueSlb, WhenAllocatingSlbBufferThenCorrectSizeIsAllocated, DeviceEnqueueSupport) {
    std::unique_ptr<MockDeviceQueueHw<FamilyType>> mockDeviceQueueHw(new MockDeviceQueueHw<FamilyType>(pContext, device, deviceQueueProperties::minimumProperties[0]));

    LinearStream *slbCS = mockDeviceQueueHw->getSlbCS();
    size_t expectedSize = (mockDeviceQueueHw->getMinimumSlbSize() + mockDeviceQueueHw->getWaCommandsSize()) * 128;
    expectedSize += sizeof(typename FamilyType::MI_BATCH_BUFFER_START);
    expectedSize = alignUp(expectedSize, MemoryConstants::pageSize);

    expectedSize += MockDeviceQueueHw<FamilyType>::getExecutionModelCleanupSectionSize();
    expectedSize += (4 * MemoryConstants::pageSize);

    EXPECT_LE(expectedSize, slbCS->getAvailableSpace());
}

HWTEST2_F(DeviceQueueSlb, WhenBuildingSlbAfterResetThenCmdsAreCorrect, DeviceEnqueueSupport) {
    auto mockDeviceQueueHw =
        new MockDeviceQueueHw<FamilyType>(pContext, device, deviceQueueProperties::minimumProperties[0]);
    auto mockDeviceQueueHwWithProfiling =
        new MockDeviceQueueHw<FamilyType>(pContext, device, deviceQueueProperties::minimumPropertiesWithProfiling[0]);

    LinearStream *slbCS = mockDeviceQueueHw->getSlbCS();
    auto expectedSize = (mockDeviceQueueHw->getMinimumSlbSize() + mockDeviceQueueHw->getWaCommandsSize()) * 128;
    expectedSize += sizeof(typename FamilyType::MI_BATCH_BUFFER_START);

    mockDeviceQueueHw->resetDeviceQueue();
    mockDeviceQueueHwWithProfiling->resetDeviceQueue();
    EXPECT_EQ(slbCS->getUsed(), expectedSize);
    EXPECT_EQ(mockDeviceQueueHwWithProfiling->getSlbCS()->getUsed(), expectedSize);

    auto cmds = mockDeviceQueueHw->expectedCmds;
    auto cmdsWithProfiling = mockDeviceQueueHwWithProfiling->expectedCmds;

    void *currCmd = slbCS->getCpuBase();
    void *currCmdWithProfiling = mockDeviceQueueHwWithProfiling->getSlbCS()->getCpuBase();
    for (size_t i = 0; i < 128; i++) {
        currCmd = compareCmds(currCmd, cmds.mediaStateFlush);
        currCmdWithProfiling = compareCmds(currCmdWithProfiling, cmdsWithProfiling.mediaStateFlush);

        if (mockDeviceQueueHw->arbCheckWa) {
            currCmd = compareCmds(currCmd, cmds.arbCheck);
            currCmdWithProfiling = compareCmds(currCmdWithProfiling, cmdsWithProfiling.arbCheck);
        }
        if (mockDeviceQueueHw->miAtomicWa) {
            currCmd = compareCmds(currCmd, cmds.miAtomic);
            currCmdWithProfiling = compareCmds(currCmdWithProfiling, cmdsWithProfiling.miAtomic);
        }

        currCmd = compareCmds(currCmd, cmds.mediaIdLoad);
        currCmdWithProfiling = compareCmds(currCmdWithProfiling, cmdsWithProfiling.mediaIdLoad);

        if (mockDeviceQueueHw->lriWa) {
            currCmd = compareCmds(currCmd, cmds.lriTrue);
            currCmdWithProfiling = compareCmds(currCmdWithProfiling, cmdsWithProfiling.lriTrue);
        }

        currCmd = compareCmds(currCmd, cmds.noopedPipeControl); // noop pipe control
        currCmdWithProfiling = compareCmds(currCmdWithProfiling, cmdsWithProfiling.pipeControl);

        if (mockDeviceQueueHw->pipeControlWa) {
            currCmd = compareCmds(currCmd, cmds.noopedPipeControl); // noop pipe control
            currCmdWithProfiling = compareCmds(currCmdWithProfiling, cmdsWithProfiling.pipeControl);
        }

        currCmd = compareCmds(currCmd, cmds.gpgpuWalker);
        currCmdWithProfiling = compareCmds(currCmdWithProfiling, cmdsWithProfiling.gpgpuWalker);

        currCmd = compareCmds(currCmd, cmds.mediaStateFlush);
        currCmdWithProfiling = compareCmds(currCmdWithProfiling, cmdsWithProfiling.mediaStateFlush);

        if (mockDeviceQueueHw->arbCheckWa) {
            currCmd = compareCmds(currCmd, cmds.arbCheck);
            currCmdWithProfiling = compareCmds(currCmdWithProfiling, cmdsWithProfiling.arbCheck);
        }

        currCmd = compareCmds(currCmd, cmds.pipeControl);
        currCmdWithProfiling = compareCmds(currCmdWithProfiling, cmdsWithProfiling.pipeControl);

        if (mockDeviceQueueHw->pipeControlWa) {
            currCmd = compareCmds(currCmd, cmds.pipeControl);
            currCmdWithProfiling = compareCmds(currCmdWithProfiling, cmdsWithProfiling.pipeControl);
        }

        if (mockDeviceQueueHw->lriWa) {
            currCmd = compareCmds(currCmd, cmds.lriFalse);
            currCmdWithProfiling = compareCmds(currCmdWithProfiling, cmdsWithProfiling.lriFalse);
        }

        currCmd = compareCmdsWithSize(currCmd, cmds.prefetch, DeviceQueueHw<FamilyType>::getCSPrefetchSize());
        currCmdWithProfiling = compareCmdsWithSize(currCmdWithProfiling, cmdsWithProfiling.prefetch, DeviceQueueHw<FamilyType>::getCSPrefetchSize());
    }

    currCmd = compareCmds(currCmd, cmds.bbStart);
    currCmdWithProfiling = compareCmds(currCmdWithProfiling, mockDeviceQueueHwWithProfiling->expectedCmds.bbStart);

    delete mockDeviceQueueHw;
    delete mockDeviceQueueHwWithProfiling;
}

HWTEST2_F(DeviceQueueSlb, WhenBuildingSlbThenOffsetIsCorrect, DeviceEnqueueSupport) {
    auto mockDeviceQueueHw = new MockDeviceQueueHw<FamilyType>(pContext, device,
                                                               deviceQueueProperties::minimumProperties[0]);

    auto slb = mockDeviceQueueHw->getSlbBuffer();
    auto commandsSize = mockDeviceQueueHw->getMinimumSlbSize() + mockDeviceQueueHw->getWaCommandsSize();
    auto slbCopy = malloc(slb->getUnderlyingBufferSize());
    memset(slb->getUnderlyingBuffer(), 0xFE, slb->getUnderlyingBufferSize());
    memcpy(slbCopy, slb->getUnderlyingBuffer(), slb->getUnderlyingBufferSize());

    auto igilCmdQueue = reinterpret_cast<IGIL_CommandQueue *>(mockDeviceQueueHw->getQueueBuffer()->getUnderlyingBuffer());

    // slbEndOffset < commandsSize * 128
    // always fill only 1 enqueue (after offset)
    auto offset = static_cast<int>(commandsSize) * 50;
    igilCmdQueue->m_controls.m_SLBENDoffsetInBytes = offset;
    mockDeviceQueueHw->resetDeviceQueue();
    EXPECT_EQ(0, memcmp(slb->getUnderlyingBuffer(), slbCopy, offset)); // dont touch memory before offset
    EXPECT_NE(0, memcmp(ptrOffset(slb->getUnderlyingBuffer(), offset),
                        slbCopy, commandsSize)); // change 1 enqueue
    EXPECT_EQ(0, memcmp(ptrOffset(slb->getUnderlyingBuffer(), offset + commandsSize),
                        slbCopy, offset)); // dont touch memory after (offset + 1 enqueue)
    compareCmds(ptrOffset(slb->getUnderlyingBuffer(), commandsSize * 128),
                mockDeviceQueueHw->expectedCmds.bbStart); // bbStart always on the same place

    // slbEndOffset == commandsSize * 128
    // dont fill commands
    memset(slb->getUnderlyingBuffer(), 0xFEFEFEFE, slb->getUnderlyingBufferSize());
    offset = static_cast<int>(commandsSize) * 128;
    igilCmdQueue->m_controls.m_SLBENDoffsetInBytes = static_cast<int>(commandsSize);
    mockDeviceQueueHw->resetDeviceQueue();
    EXPECT_EQ(0, memcmp(slb->getUnderlyingBuffer(), slbCopy, commandsSize * 128)); // dont touch memory for enqueues
    compareCmds(ptrOffset(slb->getUnderlyingBuffer(), commandsSize * 128),
                mockDeviceQueueHw->expectedCmds.bbStart); // bbStart always in the same place

    delete mockDeviceQueueHw;
    free(slbCopy);
}

HWTEST2_F(DeviceQueueSlb, WhenBuildingSlbThenCleanupSectionIsCorrect, DeviceEnqueueSupport) {
    using MI_BATCH_BUFFER_START = typename FamilyType::MI_BATCH_BUFFER_START;
    using MI_BATCH_BUFFER_END = typename FamilyType::MI_BATCH_BUFFER_END;
    using PIPE_CONTROL = typename FamilyType::PIPE_CONTROL;

    auto mockDeviceQueueHw = new MockDeviceQueueHw<FamilyType>(pContext, device, deviceQueueProperties::minimumProperties[0]);
    auto commandsSize = mockDeviceQueueHw->getMinimumSlbSize() + mockDeviceQueueHw->getWaCommandsSize();
    auto igilCmdQueue = reinterpret_cast<IGIL_CommandQueue *>(mockDeviceQueueHw->getQueueBuffer()->getUnderlyingBuffer());
    MockParentKernel *mockParentKernel = MockParentKernel::create(*pContext);
    uint32_t taskCount = 7;

    mockDeviceQueueHw->buildSlbDummyCommands();
    uint64_t tagAddress = 0x123450000;
    mockDeviceQueueHw->addExecutionModelCleanUpSection(mockParentKernel, nullptr, tagAddress, taskCount);

    HardwareParse hwParser;
    auto *slbCS = mockDeviceQueueHw->getSlbCS();
    size_t cleanupSectionOffset = alignUp(mockDeviceQueueHw->numberOfDeviceEnqueues * commandsSize + sizeof(MI_BATCH_BUFFER_START), MemoryConstants::pageSize);
    size_t cleanupSectionOffsetToParse = cleanupSectionOffset;

    size_t slbUsed = slbCS->getUsed();
    slbUsed = alignUp(slbUsed, MemoryConstants::pageSize);
    size_t slbMax = slbCS->getMaxAvailableSpace();

    // 4 pages padding expected after cleanup section
    EXPECT_LE(4 * MemoryConstants::pageSize, slbMax - slbUsed);

    if (mockParentKernel->getKernelInfo().kernelDescriptor.kernelAttributes.flags.usesFencesForReadWriteImages) {
        cleanupSectionOffsetToParse += GpgpuWalkerHelper<FamilyType>::getSizeForWADisableLSQCROPERFforOCL(mockParentKernel) / 2;
    }

    hwParser.parseCommands<FamilyType>(*slbCS, cleanupSectionOffsetToParse);
    hwParser.findHardwareCommands<FamilyType>();

    uint64_t cleanUpSectionAddress = mockDeviceQueueHw->getSlbBuffer()->getGpuAddress() + cleanupSectionOffset;
    EXPECT_EQ(cleanUpSectionAddress, igilCmdQueue->m_controls.m_CleanupSectionAddress);
    EXPECT_EQ(slbCS->getUsed() - cleanupSectionOffset, igilCmdQueue->m_controls.m_CleanupSectionSize);

    auto pipeControlItor = find<PIPE_CONTROL *>(hwParser.cmdList.begin(), hwParser.cmdList.end());
    EXPECT_NE(hwParser.cmdList.end(), pipeControlItor);

    bool tagWriteFound = false;
    while (auto pipeControlCmd = genCmdCast<PIPE_CONTROL *>(*(++pipeControlItor))) {
        if (pipeControlCmd->getPostSyncOperation() == PIPE_CONTROL::POST_SYNC_OPERATION_WRITE_IMMEDIATE_DATA) {
            if (tagAddress == NEO::UnitTestHelper<FamilyType>::getPipeControlPostSyncAddress(*pipeControlCmd)) {
                tagWriteFound = true;
                break;
            }
        }
    }

    EXPECT_TRUE(tagWriteFound);

    auto bbEndItor = find<MI_BATCH_BUFFER_END *>(hwParser.cmdList.begin(), hwParser.cmdList.end());
    EXPECT_NE(hwParser.cmdList.end(), bbEndItor);
    MI_BATCH_BUFFER_END *bbEnd = (MI_BATCH_BUFFER_END *)*bbEndItor;
    uint64_t bbEndAddres = (uint64_t)bbEnd;

    EXPECT_LE((uint64_t)mockDeviceQueueHw->getSlbBuffer()->getUnderlyingBuffer() + cleanupSectionOffset, bbEndAddres);

    delete mockParentKernel;
    delete mockDeviceQueueHw;
}

HWTEST2_F(DeviceQueueSlb, GivenProfilingWhenBuildingSlbThenEmCleanupSectionIsAdded, DeviceEnqueueSupport) {
    using MI_BATCH_BUFFER_START = typename FamilyType::MI_BATCH_BUFFER_START;
    using MI_BATCH_BUFFER_END = typename FamilyType::MI_BATCH_BUFFER_END;
    using PIPE_CONTROL = typename FamilyType::PIPE_CONTROL;
    using MI_STORE_REGISTER_MEM = typename FamilyType::MI_STORE_REGISTER_MEM;
    using MI_LOAD_REGISTER_IMM = typename FamilyType::MI_LOAD_REGISTER_IMM;

    auto mockDeviceQueueHw = new MockDeviceQueueHw<FamilyType>(pContext, device, deviceQueueProperties::minimumProperties[0]);
    auto commandsSize = mockDeviceQueueHw->getMinimumSlbSize() + mockDeviceQueueHw->getWaCommandsSize();
    auto igilCmdQueue = reinterpret_cast<IGIL_CommandQueue *>(mockDeviceQueueHw->getQueueBuffer()->getUnderlyingBuffer());
    MockParentKernel *mockParentKernel = MockParentKernel::create(*pContext);
    uint32_t taskCount = 7;

    auto hwTimeStamp = pCommandQueue->getGpgpuCommandStreamReceiver().getEventTsAllocator()->getTag();
    mockDeviceQueueHw->buildSlbDummyCommands();
    mockDeviceQueueHw->addExecutionModelCleanUpSection(mockParentKernel, hwTimeStamp, 0x123, taskCount);

    uint64_t eventTimestampAddr = igilCmdQueue->m_controls.m_EventTimestampAddress;
    uint64_t contextCompleteAddr = hwTimeStamp->getGpuAddress() + offsetof(HwTimeStamps, ContextCompleteTS);
    EXPECT_EQ(contextCompleteAddr, eventTimestampAddr);

    HardwareParse hwParser;
    auto *slbCS = mockDeviceQueueHw->getSlbCS();
    size_t cleanupSectionOffset = alignUp(mockDeviceQueueHw->numberOfDeviceEnqueues * commandsSize + sizeof(MI_BATCH_BUFFER_START), MemoryConstants::pageSize);
    size_t cleanupSectionOffsetToParse = cleanupSectionOffset;

    hwParser.parseCommands<FamilyType>(*slbCS, cleanupSectionOffsetToParse);
    hwParser.findHardwareCommands<FamilyType>();

    uint64_t cleanUpSectionAddress = mockDeviceQueueHw->getSlbBuffer()->getGpuAddress() + cleanupSectionOffset;
    EXPECT_EQ(cleanUpSectionAddress, igilCmdQueue->m_controls.m_CleanupSectionAddress);
    EXPECT_EQ(slbCS->getUsed() - cleanupSectionOffset, igilCmdQueue->m_controls.m_CleanupSectionSize);

    auto pipeControlItor = find<PIPE_CONTROL *>(hwParser.cmdList.begin(), hwParser.cmdList.end());

    if (mockParentKernel->getKernelInfo().kernelDescriptor.kernelAttributes.flags.usesFencesForReadWriteImages && GpgpuWalkerHelper<FamilyType>::getSizeForWADisableLSQCROPERFforOCL(mockParentKernel) > 0) {
        auto loadRegImmItor = find<MI_LOAD_REGISTER_IMM *>(hwParser.cmdList.begin(), hwParser.cmdList.end());
        EXPECT_NE(hwParser.cmdList.end(), loadRegImmItor);

        pipeControlItor = find<PIPE_CONTROL *>(loadRegImmItor, hwParser.cmdList.end());
        pipeControlItor++;
    }

    EXPECT_NE(hwParser.cmdList.end(), pipeControlItor);

    PIPE_CONTROL *pipeControl = (PIPE_CONTROL *)*pipeControlItor;
    EXPECT_NE(0u, pipeControl->getCommandStreamerStallEnable());

    auto loadRegImmItor = find<MI_LOAD_REGISTER_IMM *>(pipeControlItor, hwParser.cmdList.end());

    ASSERT_NE(hwParser.cmdList.end(), loadRegImmItor);

    MI_LOAD_REGISTER_IMM *loadRegImm = (MI_LOAD_REGISTER_IMM *)*loadRegImmItor;

    EXPECT_EQ(0x2248u, loadRegImm->getRegisterOffset());
    EXPECT_EQ(0u, loadRegImm->getDataDword());

    pipeControlItor++;
    EXPECT_NE(hwParser.cmdList.end(), pipeControlItor);

    auto bbEndItor = find<MI_BATCH_BUFFER_END *>(hwParser.cmdList.begin(), hwParser.cmdList.end());
    EXPECT_NE(hwParser.cmdList.end(), bbEndItor);
    MI_BATCH_BUFFER_END *bbEnd = (MI_BATCH_BUFFER_END *)*bbEndItor;
    uint64_t bbEndAddres = (uint64_t)bbEnd;

    EXPECT_LE((uint64_t)mockDeviceQueueHw->getSlbBuffer()->getUnderlyingBuffer() + cleanupSectionOffset, bbEndAddres);

    delete mockParentKernel;
    delete mockDeviceQueueHw;
}

HWTEST2_F(DeviceQueueHwTest, WhenCreatingDeviceQueueThenDshBufferParamsAreCorrect, DeviceEnqueueSupport) {
    using INTERFACE_DESCRIPTOR_DATA = typename FamilyType::INTERFACE_DESCRIPTOR_DATA;

    deviceQueue = createQueueObject();
    ASSERT_NE(deviceQueue, nullptr);

    auto *devQueueHw = castToObject<DeviceQueueHw<FamilyType>>(deviceQueue);

    auto heap = devQueueHw->getIndirectHeap(IndirectHeap::DYNAMIC_STATE);

    ASSERT_NE(nullptr, heap);

    auto dshBuffer = deviceQueue->getDshBuffer()->getUnderlyingBuffer();
    auto dshBufferSize = deviceQueue->getDshBuffer()->getUnderlyingBufferSize();

    auto size = heap->getAvailableSpace();
    auto heapMemory = heap->getCpuBase();

    // ExecutionModel DSH is offseted by colorCalcState, ParentKernel Interface Descriptor Data is located in first table just after colorCalcState

    EXPECT_EQ(dshBufferSize - DeviceQueue::colorCalcStateSize, size);
    EXPECT_EQ(dshBuffer, heapMemory);
    EXPECT_EQ(ptrOffset(dshBuffer, DeviceQueue::colorCalcStateSize), heap->getSpace(0));

    delete deviceQueue;
}

HWTEST2_F(DeviceQueueHwTest, WhenCreatingDeviceQueueThenDshOffsetIsCorrect, DeviceEnqueueSupport) {
    using INTERFACE_DESCRIPTOR_DATA = typename FamilyType::INTERFACE_DESCRIPTOR_DATA;

    deviceQueue = createQueueObject();
    ASSERT_NE(deviceQueue, nullptr);

    auto *devQueueHw = castToObject<DeviceQueueHw<FamilyType>>(deviceQueue);

    size_t offsetDsh = sizeof(INTERFACE_DESCRIPTOR_DATA) * DeviceQueue::interfaceDescriptorEntries * DeviceQueue::numberOfIDTables + DeviceQueue::colorCalcStateSize;

    EXPECT_EQ(devQueueHw->getDshOffset(), offsetDsh);

    delete deviceQueue;
}

class DeviceQueueHwWithKernel : public ExecutionModelKernelFixture,
                                public ::testing::WithParamInterface<std::tuple<const char *, const char *>> {
  public:
    void SetUp() override {
        REQUIRE_DEVICE_ENQUEUE_OR_SKIP(defaultHwInfo);

        ExecutionModelKernelFixture::SetUp(std::get<0>(GetParam()), std::get<1>(GetParam()));
        cl_queue_properties properties[5] = {
            CL_QUEUE_PROPERTIES,
            CL_QUEUE_ON_DEVICE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE,
            0, 0, 0};
        cl_int errcodeRet = 0;

        clDevice = new MockClDevice{MockDevice::createWithNewExecutionEnvironment<MockDevice>(defaultHwInfo.get())};
        device = &clDevice->getDevice();
        context = new MockContext();
        ASSERT_NE(nullptr, context);

        devQueue = DeviceQueue::create(context, clDevice,
                                       *properties,
                                       errcodeRet);

        ASSERT_NE(nullptr, devQueue);
    }
    void TearDown() override {
        if (IsSkipped()) {
            return;
        }

        if (devQueue) {
            delete devQueue;
        }
        if (context) {
            delete context;
        }
        if (clDevice) {
            delete clDevice;
        }
        ExecutionModelKernelFixture::TearDown();
    }

    Device *device = nullptr;
    ClDevice *clDevice = nullptr;
    DeviceQueue *devQueue = nullptr;
    MockContext *context = nullptr;
};

HWTEST2_P(DeviceQueueHwWithKernel, WhenSetiingIUpIndirectStateThenDshIsNotUsed, DeviceEnqueueSupport) {
    EXPECT_TRUE(pKernel->isParentKernel);

    pKernel->createReflectionSurface();

    auto *devQueueHw = castToObject<DeviceQueueHw<FamilyType>>(devQueue);

    ASSERT_NE(nullptr, devQueueHw);
    auto dsh = devQueueHw->getIndirectHeap(IndirectHeap::DYNAMIC_STATE);
    ASSERT_NE(nullptr, dsh);

    size_t surfaceStateHeapSize = HardwareCommandsHelper<FamilyType>::getSshSizeForExecutionModel(const_cast<const Kernel &>(*pKernel));

    auto ssh = new IndirectHeap(alignedMalloc(surfaceStateHeapSize, MemoryConstants::pageSize), surfaceStateHeapSize);
    auto usedBeforeSSH = ssh->getUsed();
    auto usedBeforeDSH = dsh->getUsed();

    devQueueHw->setupIndirectState(*ssh, *dsh, pKernel, 1, false);
    auto usedAfterSSH = ssh->getUsed();
    auto usedAfterDSH = dsh->getUsed();

    EXPECT_GE(surfaceStateHeapSize, usedAfterSSH - usedBeforeSSH);

    EXPECT_EQ(0u, usedAfterDSH - usedBeforeDSH);

    alignedFree(ssh->getCpuBase());
    delete ssh;
}

HWTEST2_P(DeviceQueueHwWithKernel, WhenSettingUpIndirectStateThenCorrectStartBlockIdIsSet, DeviceEnqueueSupport) {
    EXPECT_TRUE(pKernel->isParentKernel);

    pKernel->createReflectionSurface();

    auto *devQueueHw = castToObject<DeviceQueueHw<FamilyType>>(devQueue);
    ASSERT_NE(nullptr, devQueueHw);
    auto dsh = devQueueHw->getIndirectHeap(IndirectHeap::DYNAMIC_STATE);
    ASSERT_NE(nullptr, dsh);

    size_t surfaceStateHeapSize = HardwareCommandsHelper<FamilyType>::getSshSizeForExecutionModel(const_cast<const Kernel &>(*pKernel));

    auto ssh = new IndirectHeap(alignedMalloc(surfaceStateHeapSize, MemoryConstants::pageSize), surfaceStateHeapSize);

    uint32_t parentCount = 4;

    devQueueHw->setupIndirectState(*ssh, *dsh, pKernel, parentCount, false);
    auto *igilQueue = reinterpret_cast<IGIL_CommandQueue *>(devQueueHw->getQueueBuffer()->getUnderlyingBuffer());

    EXPECT_EQ(parentCount, igilQueue->m_controls.m_StartBlockID);

    alignedFree(ssh->getCpuBase());
    delete ssh;
}

HWTEST2_P(DeviceQueueHwWithKernel, WhenSettingUpIndirectStateThenDshValuesAreSetCorrectly, DeviceEnqueueSupport) {
    using GPGPU_WALKER = typename FamilyType::GPGPU_WALKER;

    EXPECT_TRUE(pKernel->isParentKernel);

    pKernel->createReflectionSurface();

    MockContext mockContext;
    MockDeviceQueueHw<FamilyType> *devQueueHw = new MockDeviceQueueHw<FamilyType>(&mockContext, clDevice, deviceQueueProperties::minimumProperties[0]);
    ASSERT_NE(nullptr, devQueueHw);
    auto dsh = devQueueHw->getIndirectHeap(IndirectHeap::DYNAMIC_STATE);
    ASSERT_NE(nullptr, dsh);

    size_t surfaceStateHeapSize = HardwareCommandsHelper<FamilyType>::getSshSizeForExecutionModel(const_cast<const Kernel &>(*pKernel));

    auto ssh = new IndirectHeap(alignedMalloc(surfaceStateHeapSize, MemoryConstants::pageSize), surfaceStateHeapSize);

    uint32_t parentCount = 1;

    devQueueHw->setupIndirectState(*ssh, *dsh, pKernel, parentCount, false);
    auto *igilQueue = reinterpret_cast<IGIL_CommandQueue *>(devQueueHw->getQueueBuffer()->getUnderlyingBuffer());

    EXPECT_EQ(igilQueue->m_controls.m_DynamicHeapStart, devQueueHw->offsetDsh + alignUp((uint32_t)pKernel->getDynamicStateHeapSize(), GPGPU_WALKER::INDIRECTDATASTARTADDRESS_ALIGN_SIZE));
    EXPECT_EQ(igilQueue->m_controls.m_DynamicHeapSizeInBytes, (uint32_t)devQueueHw->getDshBuffer()->getUnderlyingBufferSize());
    EXPECT_EQ(igilQueue->m_controls.m_CurrentDSHoffset, devQueueHw->offsetDsh + alignUp((uint32_t)pKernel->getDynamicStateHeapSize(), GPGPU_WALKER::INDIRECTDATASTARTADDRESS_ALIGN_SIZE));
    EXPECT_EQ(igilQueue->m_controls.m_ParentDSHOffset, devQueueHw->offsetDsh);

    alignedFree(ssh->getCpuBase());
    delete ssh;
    delete devQueueHw;
}

HWTEST2_P(DeviceQueueHwWithKernel, GivenHasBarriersSetWhenCallingSetupIndirectStateThenAllIddHaveBarriersEnabled, DeviceEnqueueSupport) {
    using GPGPU_WALKER = typename FamilyType::GPGPU_WALKER;
    using INTERFACE_DESCRIPTOR_DATA = typename FamilyType::INTERFACE_DESCRIPTOR_DATA;

    pKernel->createReflectionSurface();

    MockContext mockContext;
    auto devQueueHw = std::make_unique<MockDeviceQueueHw<FamilyType>>(&mockContext, clDevice, deviceQueueProperties::minimumProperties[0]);
    auto dsh = devQueueHw->getIndirectHeap(IndirectHeap::DYNAMIC_STATE);

    uint32_t parentCount = 1;

    auto blockManager = pKernel->getProgram()->getBlockKernelManager();
    auto iddCount = blockManager->getCount();
    for (uint32_t i = 0; i < iddCount; i++) {
        const_cast<KernelDescriptor &>(blockManager->getBlockKernelInfo(i)->kernelDescriptor).kernelAttributes.barrierCount = 1U;
    }

    auto surfaceStateHeapSize =
        HardwareCommandsHelper<FamilyType>::getSshSizeForExecutionModel(const_cast<const Kernel &>(*pKernel));
    auto ssh = std::make_unique<IndirectHeap>(alignedMalloc(surfaceStateHeapSize, MemoryConstants::pageSize), surfaceStateHeapSize);

    devQueueHw->setupIndirectState(*ssh, *dsh, pKernel, parentCount, false);

    auto iddStartPtr = static_cast<INTERFACE_DESCRIPTOR_DATA *>(ptrOffset(dsh->getCpuBase(), devQueueHw->colorCalcStateSize));
    auto iddStartIndex = parentCount;
    for (uint32_t i = 0; i < iddCount; i++) {
        EXPECT_TRUE(iddStartPtr[iddStartIndex + i].getBarrierEnable());
    }

    alignedFree(ssh->getCpuBase());
}

static const char *binaryFile = "simple_block_kernel";
static const char *KernelNames[] = {"kernel_reflection", "simple_block_kernel"};

INSTANTIATE_TEST_CASE_P(DeviceQueueHwWithKernel,
                        DeviceQueueHwWithKernel,
                        ::testing::Combine(
                            ::testing::Values(binaryFile),
                            ::testing::ValuesIn(KernelNames)));

struct TheSimplestDeviceQueueFixture : testing::Test {
    void SetUp() override {
        REQUIRE_DEVICE_ENQUEUE_OR_SKIP(defaultHwInfo);
    }
    void TearDown() override {
    }
};

HWTEST2_F(TheSimplestDeviceQueueFixture, WhenResettingDeviceQueueThenEarlyReturnValuesAreSet, DeviceEnqueueSupport) {

    DebugManagerStateRestore dbgRestorer;

    DebugManager.flags.SchedulerSimulationReturnInstance.set(3);

    auto device = std::make_unique<MockClDevice>(MockDevice::createWithNewExecutionEnvironment<MockDevice>(defaultHwInfo.get()));
    MockContext context;
    std::unique_ptr<MockDeviceQueueHw<FamilyType>> mockDeviceQueueHw(new MockDeviceQueueHw<FamilyType>(&context, device.get(), deviceQueueProperties::minimumProperties[0]));

    mockDeviceQueueHw->resetDeviceQueue();

    EXPECT_EQ(3u, mockDeviceQueueHw->getIgilQueue()->m_controls.m_SchedulerEarlyReturn);
    EXPECT_EQ(0u, mockDeviceQueueHw->getIgilQueue()->m_controls.m_SchedulerEarlyReturnCounter);
}

HWTEST2_F(TheSimplestDeviceQueueFixture, WhenAddihMediaStateClearCmdsThenCmdsAreAddedCorrectly, DeviceEnqueueSupport) {
    using PIPE_CONTROL = typename FamilyType::PIPE_CONTROL;
    using MEDIA_VFE_STATE = typename FamilyType::MEDIA_VFE_STATE;

    auto device = std::make_unique<MockClDevice>(MockDevice::createWithNewExecutionEnvironment<MockDevice>(defaultHwInfo.get()));
    MockContext context;
    std::unique_ptr<MockDeviceQueueHw<FamilyType>> mockDeviceQueueHw(new MockDeviceQueueHw<FamilyType>(&context, device.get(), deviceQueueProperties::minimumProperties[0]));

    HardwareParse hwParser;
    auto *slbCS = mockDeviceQueueHw->getSlbCS();

    mockDeviceQueueHw->addMediaStateClearCmds();

    hwParser.parseCommands<FamilyType>(*slbCS, 0);
    hwParser.findHardwareCommands<FamilyType>();

    auto pipeControlItor = find<PIPE_CONTROL *>(hwParser.cmdList.begin(), hwParser.cmdList.end());
    EXPECT_NE(hwParser.cmdList.end(), pipeControlItor);

    if (mockDeviceQueueHw->pipeControlWa) {
        pipeControlItor++;
        EXPECT_NE(hwParser.cmdList.end(), pipeControlItor);
    }

    PIPE_CONTROL *pipeControl = (PIPE_CONTROL *)*pipeControlItor;
    EXPECT_TRUE(pipeControl->getGenericMediaStateClear());

    auto mediaVfeStateItor = find<MEDIA_VFE_STATE *>(pipeControlItor, hwParser.cmdList.end());

    EXPECT_NE(hwParser.cmdList.end(), mediaVfeStateItor);
}

HWTEST2_F(TheSimplestDeviceQueueFixture, WhenAddingExecutionModelCleanupSectionThenMediaStateIsCleared, DeviceEnqueueSupport) {
    using PIPE_CONTROL = typename FamilyType::PIPE_CONTROL;
    using MEDIA_VFE_STATE = typename FamilyType::MEDIA_VFE_STATE;

    class MockDeviceQueueWithMediaStateClearRegistering : public MockDeviceQueueHw<FamilyType> {
      public:
        MockDeviceQueueWithMediaStateClearRegistering(Context *context,
                                                      ClDevice *device,
                                                      cl_queue_properties &properties) : MockDeviceQueueHw<FamilyType>(context, device, properties) {
        }

        bool addMediaStateClearCmdsCalled = false;
        void addMediaStateClearCmds() override {
            addMediaStateClearCmdsCalled = true;
        }
    };
    auto device = std::make_unique<MockClDevice>(MockDevice::createWithNewExecutionEnvironment<MockDevice>(defaultHwInfo.get()));
    MockContext context(device.get());
    std::unique_ptr<MockDeviceQueueWithMediaStateClearRegistering> mockDeviceQueueHw(new MockDeviceQueueWithMediaStateClearRegistering(&context, device.get(), deviceQueueProperties::minimumProperties[0]));

    std::unique_ptr<MockParentKernel> mockParentKernel(MockParentKernel::create(context));
    uint32_t taskCount = 7;
    mockDeviceQueueHw->buildSlbDummyCommands();

    EXPECT_FALSE(mockDeviceQueueHw->addMediaStateClearCmdsCalled);
    mockDeviceQueueHw->addExecutionModelCleanUpSection(mockParentKernel.get(), nullptr, 0x123, taskCount);
    EXPECT_TRUE(mockDeviceQueueHw->addMediaStateClearCmdsCalled);
}

HWTEST2_F(TheSimplestDeviceQueueFixture, WhenSettingMediaStateClearThenCmdsSizeIsCorrect, DeviceEnqueueSupport) {
    using PIPE_CONTROL = typename FamilyType::PIPE_CONTROL;
    using MEDIA_VFE_STATE = typename FamilyType::MEDIA_VFE_STATE;

    auto device = std::make_unique<MockClDevice>(MockDevice::createWithNewExecutionEnvironment<MockDevice>(defaultHwInfo.get()));
    MockContext context;
    std::unique_ptr<MockDeviceQueueHw<FamilyType>> mockDeviceQueueHw(new MockDeviceQueueHw<FamilyType>(&context, device.get(), deviceQueueProperties::minimumProperties[0]));

    size_t expectedSize = 2 * sizeof(PIPE_CONTROL) + sizeof(PIPE_CONTROL) + sizeof(MEDIA_VFE_STATE);
    EXPECT_EQ(expectedSize, MockDeviceQueueHw<FamilyType>::getMediaStateClearCmdsSize());
}

HWTEST2_F(TheSimplestDeviceQueueFixture, WhenSettingExecutionModelCleanupThenSectionSizeIsCorrect, DeviceEnqueueSupport) {
    using PIPE_CONTROL = typename FamilyType::PIPE_CONTROL;
    using MI_MATH_ALU_INST_INLINE = typename FamilyType::MI_MATH_ALU_INST_INLINE;
    using MI_LOAD_REGISTER_REG = typename FamilyType::MI_LOAD_REGISTER_REG;
    using MI_LOAD_REGISTER_IMM = typename FamilyType::MI_LOAD_REGISTER_IMM;
    using MI_MATH = typename FamilyType::MI_MATH;
    using MI_BATCH_BUFFER_END = typename FamilyType::MI_BATCH_BUFFER_END;

    auto device = std::make_unique<MockClDevice>(MockDevice::createWithNewExecutionEnvironment<MockDevice>(defaultHwInfo.get()));
    MockContext context;
    std::unique_ptr<MockDeviceQueueHw<FamilyType>> mockDeviceQueueHw(new MockDeviceQueueHw<FamilyType>(&context, device.get(), deviceQueueProperties::minimumProperties[0]));

    size_t expectedSize = sizeof(PIPE_CONTROL) +
                          2 * sizeof(MI_LOAD_REGISTER_REG) +
                          sizeof(MI_LOAD_REGISTER_IMM) +
                          sizeof(PIPE_CONTROL) +
                          sizeof(MI_MATH) +
                          NUM_ALU_INST_FOR_READ_MODIFY_WRITE * sizeof(MI_MATH_ALU_INST_INLINE);

    expectedSize += MockDeviceQueueHw<FamilyType>::getProfilingEndCmdsSize();
    expectedSize += MockDeviceQueueHw<FamilyType>::getMediaStateClearCmdsSize();

    expectedSize += 4 * sizeof(PIPE_CONTROL);
    expectedSize += sizeof(MI_BATCH_BUFFER_END);

    EXPECT_EQ(expectedSize, MockDeviceQueueHw<FamilyType>::getExecutionModelCleanupSectionSize());
}

HWTEST2_F(TheSimplestDeviceQueueFixture, WhenSettingProfilingEndThenCmdsSizeIsCorrect, DeviceEnqueueSupport) {
    using PIPE_CONTROL = typename FamilyType::PIPE_CONTROL;
    using MI_STORE_REGISTER_MEM = typename FamilyType::MI_STORE_REGISTER_MEM;
    using MI_LOAD_REGISTER_IMM = typename FamilyType::MI_LOAD_REGISTER_IMM;

    auto device = std::make_unique<MockClDevice>(MockDevice::createWithNewExecutionEnvironment<MockDevice>(defaultHwInfo.get()));
    MockContext context;
    std::unique_ptr<MockDeviceQueueHw<FamilyType>> mockDeviceQueueHw(new MockDeviceQueueHw<FamilyType>(&context, device.get(), deviceQueueProperties::minimumProperties[0]));

    size_t expectedSize = sizeof(PIPE_CONTROL) + sizeof(MI_STORE_REGISTER_MEM) + sizeof(MI_LOAD_REGISTER_IMM);

    EXPECT_EQ(expectedSize, MockDeviceQueueHw<FamilyType>::getProfilingEndCmdsSize());
}
