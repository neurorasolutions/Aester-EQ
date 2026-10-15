// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (c) 2026 Neurora and contributors.
#include "RootSection.h"
#include <cmath>
#include <iostream>
#include <stdexcept>

using namespace distillazione;
using Complex = std::complex<double>;
static int checks = 0;
static void check(bool condition) {
    ++checks;
    if (!condition) throw std::runtime_error("Root conversion check failed");
}
static bool near(Complex a, Complex b) { return std::abs(a-b) <= 2e-6*(1+std::abs(b)); }

int main() {
    try {
        const RootSlots empty{};
        const RootSlots realPole{Complex{-1, 0}, std::nullopt};
        const auto lowpass = sectionFromRoots(empty, realPole, 1, 1, 1);
        const RootSlots zeros{Complex{0, 0}, Complex{0, 0}};
        const RootSlots poles{Complex{-0.5, 0.8}, Complex{-0.5, -0.8}};
        const auto section = sectionFromRoots(zeros, poles, 2, 2, 2);
        const auto normalized = sectionFromRoots(empty, poles, 2, 2, 2, true);
        const RootSlots missingFirst{std::nullopt, Complex{-2, 0}};
        const auto oneRoot = sectionFromRoots(missingFirst, empty, 2, 2, 1);
        for (double w : {0.0, 0.01, 0.1, 1.0, 2.0, 10.0, 100.0}) {
            const Complex s{0, w};
            check(near(lowpass.response(w), 1.0/(s+1.0)));
            // Independent factored transfer function versus generated polynomials.
            check(near(section.response(w), 2.0*s*s / ((s-*poles[0])*(s-*poles[1]))));
            check(near(oneRoot.response(w), s+2.0));
        }
        check(std::abs(std::abs(normalized.response(1))-1) < 2e-6);
        const auto stage = sectionFromRoots(empty, empty, 2, 4, 10);
        check(near(cascadeResponse({stage, stage}, 1), {10, 0}));
        const auto identity = sectionFromRoots(poles, poles, 2, 2, 1);
        check(near(identity.response(1), {1, 0}));
        bool rejected = false;
        try { sectionFromRoots({Complex{-1, 1}, std::nullopt}, empty, 1, 1, 1); }
        catch (const std::invalid_argument&) { rejected = true; }
        check(rejected);
        std::cout << checks << " root conversion checks passed\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n'; return 1;
    }
}
