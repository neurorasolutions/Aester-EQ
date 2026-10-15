// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (c) 2026 Neurora and contributors.
#include "RootSection.h"

#include <cmath>
#include <limits>
#include <stdexcept>

namespace distillazione {
namespace {
struct Polynomial { double quadratic, linear, constant; };

Polynomial polynomial(const RootSlots& slots, unsigned count) {
    std::complex<double> linear{0, 0}, constant{1, 0};
    unsigned degree = 0;
    for (unsigned i = 0; i < count; ++i) {
        if (!slots[i]) continue;
        const auto root = *slots[i];
        if (!std::isfinite(root.real()) || !std::isfinite(root.imag()))
            throw std::invalid_argument("Roots must be finite or empty");
        if (degree == 0) {
            constant = -root;
            linear = {1, 0};
        } else {
            constant *= -root;
            linear = -*slots[0] - root;
            // The first populated slot can only be slot 0 when two roots exist.
        }
        ++degree;
    }
    const double tolerance = 1e-12 * (1 + std::abs(linear) + std::abs(constant));
    if (std::abs(linear.imag()) > tolerance || std::abs(constant.imag()) > tolerance)
        throw std::invalid_argument("Roots must produce real coefficients");
    return {degree == 2 ? 1.0 : 0.0, linear.real(), constant.real()};
}
}

AnalogSection sectionFromRoots(const RootSlots& zeros, const RootSlots& poles,
                               unsigned slots, unsigned totalSlots, double gain,
                               bool normalizeAtOne) {
    if (slots < 1 || slots > 2 || totalSlots < slots || totalSlots > 16
        || !std::isfinite(gain) || gain <= 0)
        throw std::invalid_argument("Valid slot counts and positive finite gain required");
    if (slots == 1 && (zeros[1] || poles[1]))
        throw std::invalid_argument("Second slot must be empty for a first-order section");
    const auto n = polynomial(zeros, slots);
    const auto d = polynomial(poles, slots);
    // Native 0x1854 rounds the reciprocal exponent, root gain, and square to float.
    const float exponent = 1.0f / static_cast<float>(totalSlots);
    const float rootGain = static_cast<float>(std::pow(gain, static_cast<double>(exponent)));
    const double sectionGain = slots == 2 ? static_cast<double>(rootGain * rootGain)
                                         : static_cast<double>(rootGain);
    AnalogSection result{n.quadratic * sectionGain, n.linear * sectionGain,
                         n.constant * sectionGain, d.quadratic, d.linear, d.constant};
    if (normalizeAtOne) {
        const double denominatorPower = (result.d0-result.d2)*(result.d0-result.d2)
                                      + result.d1*result.d1;
        double magnitude = 1.0;
        if (std::abs(static_cast<float>(denominatorPower)) > std::numeric_limits<float>::epsilon()) {
            const double numeratorPower = (result.n0-result.n2)*(result.n0-result.n2)
                                        + result.n1*result.n1;
            magnitude = std::sqrt(numeratorPower / denominatorPower);
        }
        if (!std::isfinite(magnitude) || magnitude <= 0)
            throw std::invalid_argument("Cannot normalize zero or nonfinite response");
        const double correction = static_cast<float>(1.0 / magnitude);
        result.n2 *= correction;
        result.n1 *= correction;
        result.n0 *= correction;
    }
    return result;
}

} // namespace distillazione
