// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (c) 2026 Neurora and contributors.
#include "AnalogResponse.h"

#include <cmath>
#include <limits>
#include <stdexcept>

namespace distillazione {

std::complex<double> AnalogSection::response(double omega) const {
    const double w2 = omega * omega;
    return std::complex<double>{n0 - w2 * n2, omega * n1}
         / std::complex<double>{d0 - w2 * d2, omega * d1};
}

double AnalogSection::observedSquaredMagnitude(double r, double scale) const {
    if (!std::isfinite(r) || !std::isfinite(scale) || scale <= 0.0)
        throw std::invalid_argument("Finite r and positive finite scale required");

    // The native branch compares n2 and d2 separately, after float conversion,
    // against FLT_EPSILON; it does not divide the coefficients.
    const auto epsilon = std::numeric_limits<float>::epsilon();
    const bool firstOrder = std::abs(static_cast<float>(n2)) <= epsilon
                         && std::abs(static_cast<float>(d2)) <= epsilon;
    const double scale2 = scale * scale;
    double a, b, c, x, y, z;
    if (firstOrder) {
        a = x = 0.0;
        b = n1 * n1;
        c = scale2 * n0 * n0;
        y = d1 * d1;
        z = scale2 * d0 * d0;
    } else {
        a = n2 * n2;
        b = scale2 * (n1 * n1 - (n2 + n2) * n0);
        c = scale2 * n0;
        c *= c;
        x = d2 * d2;
        y = scale2 * (d1 * d1 - (d2 + d2) * d0);
        z = scale2 * d0;
        z *= z;
    }
    const double r2 = r * r;
    double value = (c + r2 * r2 * a + r2 * b)
                 / (z + r2 * r2 * x + r2 * y);
    if (value <= 0.0)
        value = 0.0;
    return value;
}

std::complex<double> cascadeResponse(const std::vector<AnalogSection>& sections,
                                    double omega) {
    std::complex<double> result{1.0, 0.0};
    for (const auto& section : sections)
        result *= section.response(omega);
    return result;
}

} // namespace distillazione
