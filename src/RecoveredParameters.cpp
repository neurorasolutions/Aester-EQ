// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (c) 2026 Neurora and contributors.
#include "RecoveredParameters.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace distillazione {
namespace {
void validateType(unsigned type) {
    if (type > 12) throw std::invalid_argument("Internal type must be 0..12");
}
}

float amplitudeFromDb(float db) {
    if (!std::isfinite(db)) throw std::invalid_argument("Finite dB required");
    // 0xfa2b4 uses a double exponential followed by conversion to float.
    return static_cast<float>(std::exp(static_cast<double>(db) * 0.1151292546497023));
}

float linearFromLog2(float coordinate) {
    if (!std::isfinite(coordinate)) throw std::invalid_argument("Finite coordinate required");
    const float result = static_cast<float>(std::exp(static_cast<double>(coordinate) * 0.6931471805599453));
    if (!std::isfinite(result) || result <= 0)
        throw std::invalid_argument("Coordinate outside positive float range");
    return result;
}

float recoveredPrewarp(float angle) {
    if (!std::isfinite(angle) || angle <= 0)
        throw std::invalid_argument("Positive finite angular frequency required");
    return std::tan(std::min(angle, 3.1315927505493164f) * 0.5f);
}

float recoveredQ(float q, unsigned type) {
    validateType(type);
    if (!std::isfinite(q) || q <= 0)
        throw std::invalid_argument("Positive finite Q required");
    const bool logarithmic = type == 1 || type == 2 || type == 4 || type == 5
                          || type == 6 || type == 12;
    if (logarithmic)
        return std::exp2(((std::log(q) / 3.6888795f) * 0.5f + 0.5f) * 5.0f) * 0.125f;
    // Initialized by 0x5648, not the zero bytes of the static global.
    return q * 0.7071067690849304f;
}

float recoveredOrder(float slope, unsigned type) {
    validateType(type);
    if (!std::isfinite(slope)) throw std::invalid_argument("Finite slope required");
    if (slope > 8.01f && (type == 0 || type == 3)) return 12.0f;
    constexpr float coordinate[] = {-1, 5, 6, 7, 8, 9};
    constexpr float dbPerOctave[] = {0, 36, 48, 72, 96, 100};
    slope = std::min(slope, 8.0f);
    float db = dbPerOctave[0];
    for (int i = 1; i < 6; ++i) {
        if (slope <= coordinate[i]) {
            if (slope > coordinate[0])
                db = dbPerOctave[i-1] + (slope-coordinate[i-1])
                   / (coordinate[i]-coordinate[i-1]) * (dbPerOctave[i]-dbPerOctave[i-1]);
            break;
        }
    }
    const bool zeroMinimum = type == 0 || type == 3 || type == 8;
    const float minimum = zeroMinimum ? 0.0f : (type >= 7 && type <= 10 ? 2.0f : 1.0f);
    const float order = std::max(db / 6.0f, minimum);
    const float rounded = std::floor(order + 0.5f);
    return std::abs(order-rounded) <= 0.001f ? rounded : order;
}

} // namespace distillazione
