# Aester EQ user guide

## Band editing

A new instance starts without active bands. Double-click the graph to create a band, or use **+ Add band**. Drag a node to edit frequency/gain; use the band cards for precise frequency, gain and Q. Six cards are visible at a time; the page arrows access all 24 slots. Each card has its own enable switch. Click an inactive card header to enable it. Delete band disables that slot. Scroll on the graph changes the selected band's Q.

Choose a shape from its button. Low/High Cut slopes are selectable from 6, 12, 24, 36, 48, 72 or 96 dB/oct. The slope control is disabled on shapes where it does not apply. Right-click a node for band actions.

## Dynamic and Spectral

The mode button above the cards chooses Static, Dynamic or Spectral. Dynamic exposes threshold, range, attack and release in the row below the cards. Negative range reduces level when triggered; positive range expands it. Band detection follows the band's frequency region. Free detection permits separate low/high cuts.

External key requires both the per-band switch and a sidechain source routed by the DAW. A silent or missing external key must not be treated as the main input. Spectral operation uses per-frequency analysis and has additional latency. Detector audition, automatic threshold and an independent Spectral Free detector are not implemented.

## Stereo placement

Stereo applies the band to both channels. Left/Right affects one channel. Mid/Side operates on the sum/difference components. Mono ignores processing that only addresses Right or Side.

## Global processing

Live uses IIR filters and has zero added delay in Static/Dynamic operation. Analog FIR samples the complex filter response at an elevated design rate and adds 320 samples. Linear FIR retains magnitude in a symmetric kernel; larger resolutions improve low-frequency resolution and increase latency/pre-ringing. Spectral adds 1024 samples to the chosen phase path.

Mix is a linear dry/wet blend. Dry is aligned to the reported latency, including Spectral and FIR delays. Mix also blends the wet path's output level, balance and polarity toward dry. A 50% mix is not equivalent to halving every band's dB gain: complex filter responses sum. Changing Mix is smoothed over 20 ms.

The graph displays wet EQ response before Mix. It does not plot every phase-dependent dry/wet cancellation.

## Analyzer and output

Analyzer selects Pre, Post, both, or Off, with optional sidechain display and tilt. Freeze holds the current spectrum; Resume starts display updates again. Audio, gain reduction and meters continue. Freeze is editor-local, not a stored DSP parameter.

The right-hand rail contains persistent output level, balance, L/R-M/S mode, polarity, Mix and phase controls. The footer reports processing latency. Output settings are not a limiter.

## Presets and history

The preset menu saves/loads state, resets parameters or toggles bypass. Undo/Redo and A/B are local to the open editor. Preset/state persistence is separate from that local history. Output meters use peak retention and time-based decay.
