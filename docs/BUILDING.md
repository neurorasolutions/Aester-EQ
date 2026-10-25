# Building Aester EQ

JUCE 9.0.4 modules and build tools are vendored. Use CMake 3.22+, C++17 and Xcode Command Line Tools on macOS, or Visual Studio 2022 C++ tools/Windows SDK on Windows.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j2
ctest --test-dir build -C Release --output-on-failure
```

For Visual Studio use `cmake -S . -B build -A x64`. Outputs are in `build/DistillazioneEQ_artefacts/Release`: AU/Standalone on Mac, VST3/Standalone on Windows. The internal CMake target name is retained; product identifiers are unique to Aester EQ.

The published Mac binary targets arm64. No Intel build is claimed. A Windows GitHub workflow builds, tests and creates an NSIS installer. No distribution signing certificate is configured.

## Measurements

```sh
build/characterize docs/data/aester-response.csv
```

The program renders 55 single-product sine cases at 48 kHz, with .05 peak input amplitude, one second per render and analysis of the final half second. On Visual Studio the executable is `build/Release/characterize.exe`.

## Packaging

```sh
python3 packaging/build_release.py --build-dir build
```

This creates an ad hoc signed Mac AU/app, PKG, complete source archive and verified DMG. Windows uses `packaging/windows/installer.nsi`. Neither package installs or migrates other DropDownAcademy products.

Math-only builds use `-DBUILD_PLUGIN=OFF` and run six suites. Optional AAX Native development builds use `-DAESTER_BUILD_AAX=ON`; see [AAX status](AAX.md). AAX is not part of the user installers.

`tools/plot_measurements.py` optionally regenerates the measured-response figure using Matplotlib; it is not a build dependency.
