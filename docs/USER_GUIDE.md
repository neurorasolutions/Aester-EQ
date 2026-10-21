# Aester EQ user guide

## Band editing

A new instance starts without active bands. Double-click the graph to create a band, or use **+ Add band**. Drag a node to edit frequency/gain; use its floating controls for precise frequency, gain and Q. The panel stays still while turning a knob and follows the selected frequency when released. Delete disables that slot. Scroll on the graph changes the selected band's Q. The Size menu and lower-right corner resize the editor, which starts at 1000 x 700. See [Focus UI and monitoring](FOCUS_UI.md) for Solo and output controls.

Choose a shape from its button. Low/High Cut slopes are selectable from 6, 12, 24, 36, 48, 72 or 96 dB/oct. The slope control is disabled on shapes where it does not apply. Right-click a node for band actions.

## Dynamic and Spectral

The floating panel's mode button chooses Static, Dynamic or Spectral. Dynamic exposes threshold, range, attack and release in the expanded floating panel. Negative range reduces level when triggered; positive range expands it. Band detection follows the band's frequency region. Free detection permits separate low/high cuts.

External key requires both the per-band switch and a sidechain source routed by the DAW. A silent or missing external key must not be treated as the main input. Spectral operation uses per-frequency analysis and has additional latency. Detector audition, automatic threshold and an independent Spectral Free detector are not implemented.

## Stereo placement

Stereo applies the band to both channels. Left/Right affects one channel. Mid/Side operates on the sum/difference components. Mono ignores processing that only addresses Right or Side.

## Global processing

Live uses IIR filters and has zero added delay in Static/Dynamic operation. Analog FIR samples the complex filter response at an elevated design rate and adds 320 samples. Linear FIR retains magnitude in a symmetric kernel; larger resolutions improve low-frequency resolution and increase latency/pre-ringing. Spectral adds 1024 samples to the chosen phase path.

Mix is a linear dry/wet blend. Dry is aligned to the reported latency, including Spectral and FIR delays. Mix blends the wet path's output level toward dry. Global balance, polarity and the independent Mid/Side controls operate after Mix, including at 0% Mix; bypass releases them. A 50% mix is not equivalent to halving every band's dB gain: complex filter responses sum. Changing Mix is smoothed over 20 ms.

The graph displays wet EQ response before Mix. It does not plot every phase-dependent dry/wet cancellation.

## Analyzer and output

Analyzer selects Pre, Post, both, or Off, with optional sidechain display and tilt. Freeze holds the current spectrum; Resume starts display updates again. Audio, gain reduction and meters continue. Freeze is editor-local, not a stored DSP parameter.

The footer contains rotary Output, Balance and Mix controls, plus the L/R-Mid/Side menu, global polarity and phase settings. Choosing Mid/Side automatically reveals independent Mid/Side gain and polarity controls. The footer reports processing latency. Output settings are not a limiter.

## Presets and history

The preset menu saves/loads state, resets parameters or toggles bypass. Undo/Redo and A/B are local to the open editor. Preset/state persistence is separate from that local history. Output meters use peak retention and time-based decay.
