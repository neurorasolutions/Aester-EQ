// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (c) 2026 Neurora and contributors.
#include "RecoveredParameters.h"
#include <cmath>
#include <iostream>
#include <stdexcept>

static int checks = 0;
static void check(bool ok) {
    ++checks;
    if (!ok) throw std::runtime_error("Recovered parameter check failed");
}
static bool near(float a, float b) { return std::abs(a-b) < 1e-5f; }

int main() {
    try {
        using namespace distillazione;
        check(near(amplitudeFromDb(0), 1));
        check(near(amplitudeFromDb(20), 10));
        check(near(amplitudeFromDb(-20), 0.1f));
        check(near(amplitudeFromDb(12)*amplitudeFromDb(-12), 1));
        check(near(linearFromLog2(0), 1));
        check(near(linearFromLog2(10), 1024));
        check(near(linearFromLog2(-2), 0.25f));
        check(near(recoveredPrewarp(0.5f), std::tan(0.25f)));
        check(recoveredPrewarp(4) == recoveredPrewarp(100));
        const float order[] = {0, 1, 2, 3, 4, 5, 6, 8, 12, 16};
        for (int i = 0; i < 10; ++i) check(near(recoveredOrder(float(i-1), 0), order[i]));
        check(near(recoveredOrder(5.5f, 0), 7));
        check(near(recoveredOrder(0.25f, 0), 1.25f));
        check(near(recoveredOrder(9, 0), 12));
        check(near(recoveredOrder(9, 3), 12));
        check(near(recoveredOrder(9, 1), 16));
        check(near(recoveredOrder(-1, 1), 1));
        check(near(recoveredOrder(-1, 7), 2));
        check(near(recoveredOrder(-1, 8), 0));
        for (unsigned type = 0; type <= 12; ++type) {
            check(near(recoveredQ(1, type), std::sqrt(0.5f)));
            check(recoveredQ(2, type) > recoveredQ(1, type));
        }
        check(near(recoveredQ(0.025f, 1), 0.125f));
        check(near(recoveredQ(40, 1), 4));
        bool rejected = false;
        try { recoveredQ(0, 0); } catch (const std::invalid_argument&) { rejected = true; }
        check(rejected);
        std::cout << checks << " parameter checks passed\n";
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n'; return 1;
    }
}
