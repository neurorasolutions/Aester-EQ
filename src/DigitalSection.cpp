// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (c) 2026 Neurora and contributors.
#include "DigitalSection.h"
#include <cmath>
#include <limits>
#include <stdexcept>

namespace distillazione {

std::complex<double> DigitalSection::response(double theta) const {
    const auto delay = std::polar(1.0, -theta);
    return (b0 + b1*delay + b2*delay*delay) / (1.0 + a1*delay + a2*delay*delay);
}

DigitalSection bilinearSection(const AnalogSection& s, double k) {
    if (!std::isfinite(k) || k <= 0)
        throw std::invalid_argument("Positive finite transform scale required");
    const auto epsilon = std::numeric_limits<float>::epsilon();
    const bool firstOrder = std::abs(static_cast<float>(s.n2)) <= epsilon
                         && std::abs(static_cast<float>(s.d2)) <= epsilon;
    double denominator;
    DigitalSection result;
    if (firstOrder) {
        const double n = s.n0*k, d = s.d0*k;
        denominator = d+s.d1;
        const double inv = 1.0/denominator;
        result = {(n+s.n1)*inv, (n-s.n1)*inv, 0, (d-s.d1)*inv, 0};
    } else {
        const double n1 = s.n1*k, d1 = s.d1*k;
        const double n0 = k*k*s.n0, d0 = k*k*s.d0;
        const double nSum = s.n2+n0, dSum = s.d2+d0;
        denominator = d1+dSum;
        const double inv = 1.0/denominator;
        result = {(n1+nSum)*inv, (n0-s.n2)*(inv+inv), (nSum-n1)*inv,
                  (d0-s.d2)*(inv+inv), (dSum-d1)*inv};
    }
    if (!std::isfinite(denominator) || denominator == 0
        || !std::isfinite(result.b0) || !std::isfinite(result.b1)
        || !std::isfinite(result.b2) || !std::isfinite(result.a1) || !std::isfinite(result.a2))
        throw std::invalid_argument("Degenerate or nonfinite section coefficients");
    return result;
}

double DigitalSectionState::process(double input, const DigitalSection& c) noexcept {
    const double output = c.b0*input+state1;
    state1 = c.b1*input-c.a1*output+state2;
    state2 = c.b2*input-c.a2*output;
    return output;
}

} // namespace distillazione
