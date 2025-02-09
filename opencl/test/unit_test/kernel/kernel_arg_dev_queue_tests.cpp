/*
 * Copyright (C) 2018-2021 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "opencl/test/unit_test/fixtures/cl_device_fixture.h"
#include "opencl/test/unit_test/fixtures/device_host_queue_fixture.h"
#include "opencl/test/unit_test/fixtures/device_queue_matcher.h"
#include "opencl/test/unit_test/mocks/mock_buffer.h"
#include "opencl/test/unit_test/mocks/mock_kernel.h"
#include "opencl/test/unit_test/mocks/mock_program.h"

using namespace NEO;
using namespace DeviceHostQueue;

struct KernelArgDevQueueTest : public DeviceHostQueueFixture<DeviceQueue> {
  protected:
    void SetUp() override {
        DeviceHostQueueFixture<DeviceQueue>::SetUp();
        REQUIRE_DEVICE_ENQUEUE_OR_SKIP(pDevice);

        pDeviceQueue = createQueueObject();

        pKernelInfo = std::make_unique<MockKernelInfo>();
        pKernelInfo->kernelDescriptor.kernelAttributes.simdSize = 1;

        pKernelInfo->addArgDevQueue(0, crossThreadOffset, 4);

        program = std::make_unique<MockProgram>(toClDeviceVector(*pDevice));
        pKernel = new MockKernel(program.get(), *pKernelInfo, *pDevice);
        ASSERT_EQ(CL_SUCCESS, pKernel->initialize());

        uint8_t pCrossThreadData[crossThreadDataSize];
        memset(pCrossThreadData, crossThreadDataInit, sizeof(pCrossThreadData));
        pKernel->setCrossThreadData(pCrossThreadData, sizeof(pCrossThreadData));
    }

    void TearDown() override {
        delete pKernel;

        delete pDeviceQueue;

        DeviceHostQueueFixture<DeviceQueue>::TearDown();
    }

    bool crossThreadDataUnchanged() {
        for (uint32_t i = 0; i < crossThreadDataSize; i++) {
            if (pKernel->mockCrossThreadData[i] != crossThreadDataInit) {
                return false;
            }
        }

        return true;
    }

    static const uint32_t crossThreadDataSize = 0x10;
    static const char crossThreadDataInit = 0x7e;
    const CrossThreadDataOffset crossThreadOffset = 0x4;

    std::unique_ptr<MockProgram> program;
    DeviceQueue *pDeviceQueue = nullptr;
    MockKernel *pKernel = nullptr;
    std::unique_ptr<MockKernelInfo> pKernelInfo;
};

HWTEST2_F(KernelArgDevQueueTest, GivenKernelWithDevQueueArgWhenSettingArgHandleThenCorrectHandleIsSet, DeviceEnqueueSupport) {
    EXPECT_EQ(pKernel->kernelArgHandlers[0], &Kernel::setArgDevQueue);
}

HWTEST2_F(KernelArgDevQueueTest, GivenDeviceQueueWhenSettingArgDevQueueThenCorrectlyPatched, DeviceEnqueueSupport) {
    auto clDeviceQueue = static_cast<cl_command_queue>(pDeviceQueue);

    auto ret = pKernel->setArgDevQueue(0, sizeof(cl_command_queue), &clDeviceQueue);
    EXPECT_EQ(ret, CL_SUCCESS);

    auto gpuAddress = static_cast<uint32_t>(pDeviceQueue->getQueueBuffer()->getGpuAddressToPatch());
    auto patchLocation = ptrOffset(pKernel->mockCrossThreadData.data(), crossThreadOffset);
    EXPECT_EQ(*(reinterpret_cast<uint32_t *>(patchLocation)), gpuAddress);
}

HWTEST2_F(KernelArgDevQueueTest, GivenCommandQueueWhenSettingArgDevQueueThenInvalidDeviceQueueErrorIsReturned, DeviceEnqueueSupport) {
    auto clCmdQueue = static_cast<cl_command_queue>(pCommandQueue);

    auto ret = pKernel->setArgDevQueue(0, sizeof(cl_command_queue), &clCmdQueue);
    EXPECT_EQ(ret, CL_INVALID_DEVICE_QUEUE);
    EXPECT_EQ(crossThreadDataUnchanged(), true);
}

HWTEST2_F(KernelArgDevQueueTest, GivenNonQueueObjectWhenSettingArgDevQueueThenInvalidDeviceQueueErrorIsReturned, DeviceEnqueueSupport) {
    Buffer *buffer = new MockBuffer();
    auto clBuffer = static_cast<cl_mem>(buffer);

    auto ret = pKernel->setArgDevQueue(0, sizeof(cl_command_queue), &clBuffer);
    EXPECT_EQ(ret, CL_INVALID_DEVICE_QUEUE);
    EXPECT_EQ(crossThreadDataUnchanged(), true);

    delete buffer;
}

HWTEST2_F(KernelArgDevQueueTest, GivenInvalidQueueWhenSettingArgDevQueueThenInvalidDeviceQueueErrorIsReturned, DeviceEnqueueSupport) {
    char *pFakeDeviceQueue = new char[sizeof(DeviceQueue)];
    auto clFakeDeviceQueue = reinterpret_cast<cl_command_queue *>(pFakeDeviceQueue);

    auto ret = pKernel->setArgDevQueue(0, sizeof(cl_command_queue), &clFakeDeviceQueue);
    EXPECT_EQ(ret, CL_INVALID_DEVICE_QUEUE);
    EXPECT_EQ(crossThreadDataUnchanged(), true);

    delete[] pFakeDeviceQueue;
}

HWTEST2_F(KernelArgDevQueueTest, GivenNullDeviceQueueWhenSettingArgDevQueueThenInvalidArgValueErrorIsReturned, DeviceEnqueueSupport) {
    auto ret = pKernel->setArgDevQueue(0, sizeof(cl_command_queue), nullptr);
    EXPECT_EQ(ret, CL_INVALID_ARG_VALUE);
    EXPECT_EQ(crossThreadDataUnchanged(), true);
}

HWTEST2_F(KernelArgDevQueueTest, GivenInvalidSizeWhenSettingArgDevQueueThenInvalidArgSizeErrorIsReturned, DeviceEnqueueSupport) {
    auto clDeviceQueue = static_cast<cl_command_queue>(pDeviceQueue);

    auto ret = pKernel->setArgDevQueue(0, sizeof(cl_command_queue) - 1, &clDeviceQueue);
    EXPECT_EQ(ret, CL_INVALID_ARG_SIZE);
    EXPECT_EQ(crossThreadDataUnchanged(), true);
}
