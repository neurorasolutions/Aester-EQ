// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (c) 2026 Neurora and contributors.
#pragma once

#include <complex>
#include <vector>

namespace distillazione {

// Analog polynomial coefficient representation.
// Coefficients are ordered s^2, s, 1. This does not process audio samples.
struct AnalogSection {
    double n2 = 1.0, n1 = 1.0, n0 = 1.0;
    double d2 = 1.0, d1 = 1.0, d0 = 1.0;

    // omega is dimensionless: no mapping from host frequency is assumed.
    std::complex<double> response(double omega) const;

    // Reconstructs scalar squared magnitude arithmetic observed at 0x21ec.
    // r / characteristicScale is omega for nondegenerate coefficients.
    // characteristicScale must be finite and positive; r finite.
    double observedSquaredMagnitude(double r, double characteristicScale) const;
};

// Reconstruction choice: exact double complex product, not the original
// SIMD phase approximation or original cascade buffer implementation.
std::complex<double> cascadeResponse(const std::vector<AnalogSection>& sections,
                                    double omega);

} // namespace distillazione
