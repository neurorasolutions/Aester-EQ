// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (c) 2026 Neurora and contributors.
#include "AnalogResponse.h"

#include <cmath>
#include <iostream>
#include <random>
#include <stdexcept>

using distillazione::AnalogSection;

static int checks = 0;
static void require(bool condition, const char* message) {
    ++checks;
    if (!condition)
        throw std::runtime_error(message);
}
static bool near(double actual, double expected, double tolerance = 1e-10) {
    return std::abs(actual - expected) <= tolerance * (1.0 + std::abs(expected));
}

int main() {
    try {
        const AnalogSection identity;
        const AnalogSection lowpass{0, 0, 1, 0, 1, 1};
        const AnalogSection highpass{0, 1, 0, 0, 1, 1};
        const AnalogSection allpass{1, -1, 1, 1, 1, 1};
        for (double w : {0.0, 0.01, 0.1, 1.0, 10.0, 100.0}) {
            require(near(std::norm(identity.response(w)), 1), "Identity");
            require(near(std::norm(allpass.response(w)), 1), "All-pass magnitude");
            require(near(std::norm(lowpass.response(w)), 1 / (1 + w*w)), "Low-pass");
            require(near(std::norm(highpass.response(w)), w*w / (1 + w*w)), "High-pass");
            require(near(std::norm(distillazione::cascadeResponse({lowpass, lowpass}, w)),
                         1 / ((1+w*w)*(1+w*w))), "Cascade multiplication");
        }
        require(near(std::arg(lowpass.response(1)), -std::acos(-1.0)/4), "Low-pass phase");
        require(distillazione::cascadeResponse({}, 1) == std::complex<double>(1, 0),
                "Empty cascade");

        // Compare independent polynomial and complex evaluation over stable
        // random sections, frequencies and scales; no binary execution involved.
        std::mt19937 engine(410);
        std::uniform_real_distribution<double> coefficient(0.25, 4.0);
        std::uniform_real_distribution<double> exponent(-3.0, 3.0);
        for (int i = 0; i < 1000; ++i) {
            AnalogSection s{coefficient(engine), coefficient(engine), coefficient(engine),
                            coefficient(engine), coefficient(engine), coefficient(engine)};
            if (i % 2 == 0) s.n2 = s.d2 = 0;
            const double scale = std::pow(10.0, exponent(engine));
            const double w = std::pow(10.0, exponent(engine));
            require(near(s.observedSquaredMagnitude(w*scale, scale), std::norm(s.response(w)),
                         2e-9), "Polynomial versus complex magnitude");
        }
        // Native near-zero branch deliberately drops tiny second-order terms.
        const AnalogSection tiny{1e-9, 1, 1, 1e-9, 2, 1};
        require(near(tiny.observedSquaredMagnitude(1, 1), 0.4), "Native epsilon branch");
        bool rejected = false;
        try { lowpass.observedSquaredMagnitude(1, 0); }
        catch (const std::invalid_argument&) { rejected = true; }
        require(rejected, "Scale validation");
        std::cout << checks << " mathematical checks passed\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
