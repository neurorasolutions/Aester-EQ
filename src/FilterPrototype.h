// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (c) 2026 Neurora and contributors.
#pragma once
#include "AnalogResponse.h"

namespace distillazione {

// Integer-order, unity-gain pole-placement subset observed in 0x478c.
// Normalized prototype only: no public shape ID, Q modification, phase mode,
// fractional-order branch or audio sample-rate mapping is assumed.
std::vector<AnalogSection> integerLowpassPrototype(unsigned order);

} // namespace distillazione
