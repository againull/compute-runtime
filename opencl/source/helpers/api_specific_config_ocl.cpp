/*
 * Copyright (C) 2020-2021 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/source/debug_settings/debug_settings_manager.h"
#include "shared/source/helpers/api_specific_config.h"

namespace NEO {
bool ApiSpecificConfig::isStatelessCompressionSupported() {
    return true;
}

bool ApiSpecificConfig::getHeapConfiguration() {
    return false;
}

bool ApiSpecificConfig::getBindlessConfiguration() {
    if (DebugManager.flags.UseBindlessMode.get() != -1) {
        return DebugManager.flags.UseBindlessMode.get();
    } else {
        return false;
    }
}

ApiSpecificConfig::ApiType ApiSpecificConfig::getApiType() {
    return ApiSpecificConfig::OCL;
}

const char *ApiSpecificConfig::getAubPrefixForSpecificApi() {
    return "ocl_";
}

uint64_t ApiSpecificConfig::getReducedMaxAllocSize(uint64_t maxAllocSize) {
    return maxAllocSize / 2;
}

} // namespace NEO
