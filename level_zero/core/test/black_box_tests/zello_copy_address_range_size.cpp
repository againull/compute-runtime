/*
 * Copyright (C) 2020-2021 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "zello_common.h"

#include <iomanip>

extern bool verbose;
bool verbose = false;

void testAppendMemoryCopyFromDeviceToHeap(ze_context_handle_t context, ze_device_handle_t &device, bool &validRet) {
    const size_t allocSize = 12 * sizeof(int);
    void *zeBuffer = nullptr;

    ze_command_queue_handle_t cmdQueue;
    ze_command_list_handle_t cmdList;

    cmdQueue = createCommandQueue(context, device, nullptr);
    SUCCESS_OR_TERMINATE(createCommandList(context, device, cmdList));

    ze_device_mem_alloc_desc_t deviceDesc = {};
    deviceDesc.stype = ZE_STRUCTURE_TYPE_DEVICE_MEM_ALLOC_DESC;
    deviceDesc.ordinal = 0;
    deviceDesc.flags = 0;
    deviceDesc.pNext = nullptr;

    SUCCESS_OR_TERMINATE(zeMemAllocDevice(context, &deviceDesc, allocSize, allocSize, device, &zeBuffer));

    // Get size of the allocation
    void *Base;
    size_t Size;
    SUCCESS_OR_TERMINATE(zeMemGetAddressRange(context, zeBuffer, &Base, &Size));

    printf("Size of the allocation returned from zeMemGetAddressRange = %zu", Size);

    int *heapBuffer = new int[Size];

    // Copy from device-allocated memory to heap.
    SUCCESS_OR_TERMINATE(zeCommandListAppendMemoryCopy(cmdList, (void *)heapBuffer, zeBuffer, Size /*allocSize*/,
                                                       nullptr, 0, nullptr));

    SUCCESS_OR_TERMINATE(zeCommandListClose(cmdList));
    SUCCESS_OR_TERMINATE(zeCommandQueueExecuteCommandLists(cmdQueue, 1, &cmdList, nullptr));
    SUCCESS_OR_TERMINATE(zeCommandQueueSynchronize(cmdQueue, std::numeric_limits<uint32_t>::max()));

    delete[] heapBuffer;
    SUCCESS_OR_TERMINATE(zeMemFree(context, zeBuffer));
    SUCCESS_OR_TERMINATE(zeCommandListDestroy(cmdList));
    SUCCESS_OR_TERMINATE(zeCommandQueueDestroy(cmdQueue));
    validRet = true;
}

int main(int argc, char *argv[]) {
    verbose = isVerbose(argc, argv);

    ze_context_handle_t context = nullptr;
    auto devices = zelloInitContextAndGetDevices(context);
    auto device = devices[0];
    bool outputValidationSuccessful = false;

    ze_device_properties_t deviceProperties = {ZE_STRUCTURE_TYPE_DEVICE_PROPERTIES};
    SUCCESS_OR_TERMINATE(zeDeviceGetProperties(device, &deviceProperties));
    std::cout << "Device : \n"
              << " * name : " << deviceProperties.name << "\n"
              << " * vendorId : " << std::hex << deviceProperties.vendorId << "\n";

    testAppendMemoryCopyFromDeviceToHeap(context, device, outputValidationSuccessful);
 
    SUCCESS_OR_TERMINATE(zeContextDestroy(context));
    std::cout << "\nZello Copy Results validation " << (outputValidationSuccessful ? "PASSED" : "FAILED") << "\n";
    return (outputValidationSuccessful ? 0 : 1);
}
