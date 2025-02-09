/*
 * Copyright (C) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/source/device_binary_format/debug_zebin.h"

#include "shared/source/device_binary_format/elf/elf_decoder.h"
#include "shared/source/device_binary_format/elf/elf_encoder.h"

namespace NEO {
namespace Debug {
using namespace Elf;

void DebugZebinCreator::createDebugZebin() {
    ElfEncoder<EI_CLASS_64> elfEncoder(false, false);
    auto &header = elfEncoder.getElfFileHeader();
    header.machine = zebin.elfFileHeader->machine;
    header.flags = zebin.elfFileHeader->flags;
    header.type = zebin.elfFileHeader->type;
    header.version = zebin.elfFileHeader->version;
    header.shStrNdx = zebin.elfFileHeader->shStrNdx;

    for (uint32_t i = 0; i < zebin.sectionHeaders.size(); i++) {
        const auto &section = zebin.sectionHeaders[i];
        auto sectionName = zebin.getSectionName(i);
        ArrayRef<const uint8_t> sectionData;

        if (auto segment = getSegmentByName(sectionName)) {
            sectionData = segment->data;
            elfEncoder.appendProgramHeaderLoad(i, segment->address, sectionData.size());
        } else {
            sectionData = section.data;
        }

        auto &sectionHeader = elfEncoder.appendSection(section.header->type, sectionName, sectionData);
        sectionHeader.link = section.header->link;
        sectionHeader.info = section.header->info;
        sectionHeader.name = section.header->name;
        sectionHeader.flags = section.header->flags;
    }
    debugZebin = elfEncoder.encode();
}

void DebugZebinCreator::applyRelocation(uint64_t addr, uint64_t value, RELOC_TYPE_ZEBIN type) {
    switch (type) {
    default:
        UNRECOVERABLE_IF(type != R_ZE_SYM_ADDR)
        *reinterpret_cast<uint64_t *>(addr) = value;
        break;
    case R_ZE_SYM_ADDR_32:
        *reinterpret_cast<uint32_t *>(addr) = static_cast<uint32_t>(value & uint32_t(-1));
        break;
    case R_ZE_SYM_ADDR_32_HI:
        *reinterpret_cast<uint32_t *>(addr) = static_cast<uint32_t>((value >> 32) & uint32_t(-1));
        break;
    }
}

void DebugZebinCreator::applyDebugRelocations() {
    std::string errors, warnings;
    auto elf = decodeElf(debugZebin, errors, warnings);

    for (const auto &reloc : elf.getDebugInfoRelocations()) {

        auto sectionName = elf.getSectionName(reloc.symbolSectionIndex);
        uint64_t sectionAddress = 0U;
        if (auto segment = getSegmentByName(sectionName)) {
            sectionAddress = segment->address;
        } else if (ConstStringRef(sectionName).startsWith(SectionsNamesZebin::debugPrefix.data())) {
            // do not offset debug symbols
        } else {
            DEBUG_BREAK_IF(true);
            continue;
        }

        auto value = sectionAddress + elf.getSymbolValue(reloc.symbolTableIndex) + reloc.addend;
        auto address = reinterpret_cast<uint64_t>(debugZebin.data()) + elf.getSectionOffset(reloc.targetSectionIndex) + reloc.offset;
        auto type = static_cast<RELOC_TYPE_ZEBIN>(reloc.relocType);
        applyRelocation(address, value, type);
    }
}

std::vector<uint8_t> createDebugZebin(ArrayRef<const uint8_t> zebinBin, const Segments &gpuSegments) {
    std::string errors, warnings;
    auto zebin = decodeElf(zebinBin, errors, warnings);
    if (false == errors.empty()) {
        return {};
    }

    auto dzc = DebugZebinCreator(zebin, gpuSegments);
    dzc.createDebugZebin();
    dzc.applyDebugRelocations();
    return dzc.getDebugZebin();
}

const Segments::Segment *DebugZebinCreator::getSegmentByName(ConstStringRef sectionName) {
    if (sectionName.startsWith(SectionsNamesZebin::textPrefix.data())) {
        auto kernelName = sectionName.substr(SectionsNamesZebin::textPrefix.length());
        auto kernelSegmentIt = segments.nameToSegMap.find(kernelName.str());
        UNRECOVERABLE_IF(kernelSegmentIt == segments.nameToSegMap.end());
        return &kernelSegmentIt->second;
    } else if (sectionName == SectionsNamesZebin::dataConst) {
        return &segments.constData;
    } else if (sectionName == SectionsNamesZebin::dataGlobal) {
        return &segments.varData;
    } else if (sectionName == SectionsNamesZebin::dataConstString) {
        return &segments.stringData;
    }
    return nullptr;
}

} // namespace Debug
} // namespace NEO
