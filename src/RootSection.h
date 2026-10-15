// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (c) 2026 Neurora and contributors.
#pragma once

#include "AnalogResponse.h"
#include <array>
#include <optional>

namespace distillazione {

// Empty slots represent the native (+infinity, 0) sentinel.
using RootSlots = std::array<std::optional<std::complex<double>>, 2>;

// Reconstructs the finite, real-coefficient path of 0x1854. rootSlotsInSection
// is 1 for the odd final section, otherwise 2. totalRootSlots determines how
// the prototype gain is distributed across all sections. No filter-name or
// frequency mapping is implied. normalizeAtOne corresponds to internal type 8.
AnalogSection sectionFromRoots(const RootSlots& zeros, const RootSlots& poles,
                               unsigned rootSlotsInSection, unsigned totalRootSlots,
                               double prototypeGain, bool normalizeAtOne = false);

} // namespace distillazione
