// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (c) 2026 Neurora and contributors.
#include "FilterPrototype.h"
#include "RootSection.h"
#include <cmath>
#include <stdexcept>

namespace distillazione {

std::vector<AnalogSection> integerLowpassPrototype(unsigned order) {
    if (order < 1 || order > 16)
        throw std::invalid_argument("Prototype order must be 1..16");
    const double pi = std::acos(-1.0);
    std::vector<AnalogSection> result;
    result.reserve((order+1)/2);
    for (unsigned i = 0; i < order/2; ++i) {
        const double angle = double(2*i+1)*pi / double(2*order);
        const std::complex<double> pole{-std::sin(angle), std::cos(angle)};
        result.push_back(sectionFromRoots({}, {pole, std::conj(pole)}, 2, order, 1));
    }
    if (order % 2)
        result.push_back(sectionFromRoots({}, {std::complex<double>{-1, 0}, std::nullopt}, 1, order, 1));
    return result;
}

} // namespace distillazione
