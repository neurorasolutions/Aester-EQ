# Aester EQ by DropDownAcademy

A 24-band equalizer for focused tonal shaping, dynamic control and frequency-selective spectral processing.

![Aester EQ](assets/aester-focus.png)

A compact, resizable response workspace uses floating controls for the selected band and fluorescent yellow accents. The footer contains rotary Balance, Mix and Output controls. Selecting Mid / Side automatically opens independent component levels and polarity controls. [Focus UI and monitoring](docs/FOCUS_UI.md).

## Processing

- Ten filter shapes and up to 24 independent bands.
- Static, Dynamic and Spectral operation per band.
- Stereo, Left, Right, Mid and Side placement.
- Internal or external sidechain detection; independent detector cuts in Dynamic mode.
- Live IIR, Analog FIR and five Linear FIR resolutions.
- **Mix:** a smoothed, latency-aligned blend between dry and processed audio.
- **Freeze:** hold the spectrum display while processing and output meters continue.
- **Solo:** temporarily audition the selected frequency region with Q-linked bandwidth.
- Independent Mid/Side output gain and polarity after Mix.
- Pre/post/sidechain spectrum, output balance, polarity, presets, Undo/Redo and editor-local A/B.

The graph displays the **wet response before Mix**. Blending dry and wet can change the resulting frequency response because filter phase is part of the sum.

## Downloads

Version 1.3.0 is the DropDownAcademy edition. The processing engine is unchanged from 1.2.0; this release adds the DropDownAcademy identity, the new installer flow and the Italian/English user manual.

| Platform | Package | Status |
|---|---|---|
| macOS Apple Silicon | `Aester-EQ-DropDownAcademy-1.3.0-macOS-arm64.dmg` | AU + standalone, ad hoc signed |
| Windows x64 | `Aester-EQ-DropDownAcademy-1.3.0-Windows-x64.exe` | VST3 + standalone, unsigned |

Save your session and close your DAW before installing. Builds are not notarized: the Mac binaries are ad hoc signed, the Windows installer is unsigned and may show a SmartScreen warning. [Installation notes](docs/INSTALLATION.md). The Italian and English manuals are in `MANUALS/`.

## Technical character

Aester's static path uses linear filters: gain and bandwidth determine the tonal change rather than an intentional saturation stage. Dynamic mode changes filter gain from a level detector; Spectral mode uses a short-time Fourier transform to make frequency-selective changes. FIR modes introduce delay and, in linear phase, pre-ringing. This is a technical description, not a claim of perceptual transparency in every setting.

Read the [single-product technical evaluation](docs/TECHNICAL_EVALUATION.md), [test procedure](docs/TESTING.md), [user guide](docs/USER_GUIDE.md) and [build instructions](docs/BUILDING.md).

## Free and open source

Produced by [DropDownAcademy](https://www.dropdownacademy.com). Contributions and reproducible bug reports are welcome. Source is licensed under AGPL-3.0-only; retain all [third-party notices](THIRD_PARTY_NOTICES.md). Project licensing does not impose an additional non-commercial restriction.

This repository contains Aester EQ source, technical documentation and platform packages.
