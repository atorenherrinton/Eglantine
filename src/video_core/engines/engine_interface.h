// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

// SPDX-FileCopyrightText: Copyright 2020 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <array>
#include <bitset>
#include <limits>
#include <vector>

#include "common/common_types.h"

namespace Core {
class System;
}

namespace Tegra::Engines {

enum class EngineTypes : u32 {
    Nv01Timer,
    KeplerCompute,
    Maxwell3D,
    Fermi2D,
    MaxwellDMA,
    KeplerMemory,
};

/// Placeholder group counts of a compute dispatch whose real ones are a GPU-written record.
inline constexpr std::array<u32, 3> GpuRecordDispatchSize{0x7e57d150, 0xd151, 0xd152};

class EngineInterface {
public:
    virtual ~EngineInterface() = default;

    /// Write the value to the register identified by method.
    virtual void CallMethod(Core::System& system, u32 method, u32 method_argument, bool is_last_call) = 0;

    virtual void CallStateMethod(Core::System& system, u32 method, u32 argument) {
        CallMethod(system, method, argument, true);
    }

    /// Write multiple values to the register identified by method.
    virtual void CallMultiMethod(Core::System& system, u32 method, const u32* base_start, u32 amount, u32 methods_pending) = 0;

    void ConsumeSink(Core::System& system) {
        if (!method_sink.empty()) {
            ConsumeSinkImpl(system);
        }
    }

    std::bitset<(std::numeric_limits<u16>::max)()> execution_mask{};
    /// A Maxwell3D whose CallMethod takes its fast path (Nx2Perf): GpuSplit's back calls it
    /// without the virtual call.
    bool fast_maxwell3d{};
    std::vector<std::pair<u32, u32>> method_sink{};
    GPUVAddr current_dma_segment;
    /// Switch 2: the next macro call's parameters are placeholders; the real ones are the words
    /// GPU work writes at this address (DmaPusher::TakeGpuRecord). The engine has the host GPU
    /// read them where it can and reads them back itself where it can't.
    GPUVAddr gpu_record{};
    bool current_dirty{};
protected:
    virtual void ConsumeSinkImpl(Core::System& system) {
        for (auto [method, value] : method_sink) {
            CallMethod(system, method, value, true);
        }
        method_sink.clear();
    }
};

} // namespace Tegra::Engines
