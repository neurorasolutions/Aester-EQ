// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (c) 2026 Neurora and contributors.
#include "FilterPrototype.h"
#include "DigitalSection.h"
#include <cmath>
#include <iostream>
#include <stdexcept>

int main() {
    using namespace distillazione;
    int checks = 0;
    try {
        for (unsigned order = 1; order <= 16; ++order) {
            const auto sections = integerLowpassPrototype(order);
            if (sections.size() != (order+1)/2) throw std::runtime_error("Section count");
            ++checks;
            for (double omega : {0.0, 0.1, 0.5, 1.0, 2.0, 10.0}) {
                const double expected = 1.0/(1.0+std::pow(omega, double(order*2)));
                if (std::abs(std::norm(cascadeResponse(sections, omega))-expected) > 1e-10)
                    throw std::runtime_error("Butterworth magnitude");
                ++checks;
            }
            // First complete path prototype -> sections -> digital coefficients
            // -> impulse samples. Stability is checked over every supported order.
            std::vector<DigitalSection> digital;
            std::vector<DigitalSectionState> states(sections.size());
            for (const auto& s : sections) digital.push_back(bilinearSection(s, 0.1));
            double dc = 0;
            for (unsigned frame = 0; frame < 8192; ++frame) {
                double sample = frame == 0 ? 1 : 0;
                for (size_t i = 0; i < digital.size(); ++i) sample = states[i].process(sample, digital[i]);
                if (!std::isfinite(sample)) throw std::runtime_error("Nonfinite impulse");
                dc += sample;
            }
            if (std::abs(dc-1) > 1e-9) throw std::runtime_error("Impulse DC sum");
            ++checks;
        }
        std::cout << checks << " prototype checks passed\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n'; return 1;
    }
}
