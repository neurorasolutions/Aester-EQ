// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (c) 2026 Neurora and contributors.
#pragma once
#include "DigitalSection.h"
#include <array>

namespace distillazione {
enum class Shape { Bell, LowShelf, LowCut, HighShelf, HighCut, Notch, BandPass, TiltShelf, FlatTilt, AllPass };
struct BandSettings {
    bool enabled = false;
    Shape shape = Shape::Bell;
    double frequency = 1000, gain = 0, q = 1;
    int slope = 1;
    int mode=0,placement=0; // Static, Dynamic, Spectral; Stereo, Left, Right, Mid, Side
    double threshold=-24,range=-6,attack=10,release=120;
    bool externalSidechain=false;
    bool freeTrigger=false;
    double triggerLow=20,triggerHigh=20000;
};
inline Shape dynamicShapeFor(Shape shape){switch(shape){case Shape::LowCut:case Shape::HighCut:case Shape::Notch:case Shape::BandPass:case Shape::AllPass:return Shape::Bell;default:return shape;}}
struct SectionBank {
    std::array<DigitalSection, 8> sections{};
    int count = 1;
    std::complex<double> response(double theta) const;
};
// Static section design for the equalizer response.
SectionBank designStaticBand(BandSettings settings, double sampleRate);
}
