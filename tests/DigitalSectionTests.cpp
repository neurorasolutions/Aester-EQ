// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (c) 2026 Neurora and contributors.
#include "DigitalSection.h"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace distillazione;
static int checks = 0;
static void check(bool ok) {
    ++checks;
    if (!ok) throw std::runtime_error("Digital section check failed");
}
static bool near(std::complex<double> a, std::complex<double> b) {
    return std::abs(a-b) <= 1e-9*(1+std::abs(b));
}

int main() {
    try {
        const double pi = std::acos(-1.0);
        for (const auto& analog : std::vector<AnalogSection>{
                {0, 0, 1, 0, 1, 1}, {0, 1, 0, 0, 1, 1},
                {1, -1, 1, 1, 1, 1}, {1, 0.7, 2, 1, 1.1, 2}}) {
            for (double k : {0.1, 0.5, 1.0, 3.0}) {
                const auto digital = bilinearSection(analog, k);
                for (double theta : {0.0, 0.1, 0.7, 1.5, 2.5, 3.0})
                    check(near(digital.response(theta), analog.response(std::tan(theta/2)/k)));

                // Render an impulse through our sample loop and independently
                // measure its DFT against the analytical transfer function.
                DigitalSectionState state;
                std::vector<double> impulse(4096);
                for (size_t i = 0; i < impulse.size(); ++i)
                    impulse[i] = state.process(i == 0 ? 1.0 : 0.0, digital);
                for (double theta : {0.0, 0.3, 1.0, 2.0, pi}) {
                    std::complex<double> measured{0, 0};
                    for (size_t i = 0; i < impulse.size(); ++i)
                        measured += impulse[i]*std::polar(1.0, -theta*double(i));
                    check(near(measured, digital.response(theta)));
                }
                state.reset();
                check(state.process(1, digital) == digital.b0);
            }
        }
        const DigitalSection identity;
        DigitalSectionState state;
        for (double sample : {0.0, 1.0, -2.0, 0.25}) check(state.process(sample, identity) == sample);
        bool rejected = false;
        try { bilinearSection({0, 0, 1, 0, 0, 0}, 1); }
        catch (const std::invalid_argument&) { rejected = true; }
        check(rejected);
        std::cout << checks << " digital section checks passed\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n'; return 1;
    }
}
