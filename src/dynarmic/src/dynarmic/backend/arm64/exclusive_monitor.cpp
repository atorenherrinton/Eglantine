// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

/* This file is part of the dynarmic project.
 * Copyright (c) 2022 MerryMage
 * SPDX-License-Identifier: 0BSD
 */

#include "dynarmic/interface/exclusive_monitor.h"

#include <algorithm>

#include "common/assert.h"

namespace Dynarmic {

ExclusiveMonitor::ExclusiveMonitor(std::size_t processor_count)
        : exclusive_addresses(processor_count, INVALID_EXCLUSIVE_ADDRESS), exclusive_values(processor_count) {}

size_t ExclusiveMonitor::GetProcessorCount() const {
    return exclusive_addresses.size();
}

void ExclusiveMonitor::Lock() {
    lock.Lock();
}

void ExclusiveMonitor::Unlock() {
    lock.Unlock();
}

bool ExclusiveMonitor::CheckAndClear(std::size_t processor_id, VAddr address) {
    const VAddr masked_address = address & RESERVATION_GRANULE_MASK;

    Lock();
    if (exclusive_addresses[processor_id] != masked_address) {
        Unlock();
        return false;
    }

    for (VAddr& other_address : exclusive_addresses) {
        if (other_address == masked_address) {
            other_address = INVALID_EXCLUSIVE_ADDRESS;
        }
    }
    return true;
}

void ExclusiveMonitor::Clear() {
    // Lock-free like ReadAndMark and DoExclusiveOperation (exclusive_monitor.h).
    for (VAddr& address : exclusive_addresses) {
        __atomic_store_n(&address, INVALID_EXCLUSIVE_ADDRESS, __ATOMIC_SEQ_CST);
    }
}

void ExclusiveMonitor::ClearProcessor(std::size_t processor_id) {
    __atomic_store_n(&exclusive_addresses[processor_id], INVALID_EXCLUSIVE_ADDRESS,
                     __ATOMIC_SEQ_CST);
}

}  // namespace Dynarmic
