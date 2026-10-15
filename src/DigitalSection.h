// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (c) 2026 Neurora and contributors.
#pragma once

#include "AnalogResponse.h"

namespace distillazione {

struct DigitalSection {
    double b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
    std::complex<double> response(double radiansPerSample) const;
};

// Recovers the conventional-coefficient branch (param_4 == 0) of 0x1f44.
// k is the caller's positive transform scale, not a sample rate or Hz value.
// For nondegenerate sections, s=(1-z^-1)/(k*(1+z^-1)).
DigitalSection bilinearSection(const AnalogSection& analog, double k);

// Our processing implementation. The original sample loop is not yet recovered.
// Coefficients are supplied externally; no allocation occurs in process().
class DigitalSectionState {
public:
    double process(double input, const DigitalSection& coefficients) noexcept;
    void reset() noexcept { state1 = state2 = 0; }
private:
    double state1 = 0, state2 = 0;
};

} // namespace distillazione
