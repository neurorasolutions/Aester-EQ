// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (c) 2026 Neurora and contributors.
#pragma once

namespace distillazione {

// Pure helpers recovered from 0x19f6c8 and its callees (arm64 4.10).
// rawType is the internal code 0..12, not yet a public filter-name enum.
// slopeControl is the internal coordinate, not a value in dB/oct.
float recoveredOrder(float slopeControl, unsigned rawType);
float recoveredQ(float q, unsigned rawType);
float amplitudeFromDb(float db);
// 0xfa0ec: base-2 log coordinate to linear value. Units are caller-defined.
float linearFromLog2(float coordinate);
// 0xfa1f8: prewarp from normalized angular frequency, with native upper cap.
float recoveredPrewarp(float radiansPerSample);

} // namespace distillazione
