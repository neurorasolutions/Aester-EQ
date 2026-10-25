# Focus UI and monitoring

These features are included in the 1.3.0 DropDownAcademy packages.

- Floating selected-band frequency/gain/Q controls, fluorescent yellow accents, narrow output meters and compact footer.
- No band controls are displayed until a band is created. A single click selects existing nodes; double-click adds a band.
- The panel follows frequency changes but stays stationary during knob drags.
- SOLO isolates a frequency region of the final processed output, rather than bypassing the other EQ bands. Bell, notch, band-pass, tilt and all-pass shapes use a unity-centre band-pass monitor whose bandwidth follows Q. Low Shelf/High Cut audition the low-frequency region; High Shelf/Low Cut audition the high-frequency region.
- Stereo placement is respected. Monitoring follows the existing processing latency and uses a 20 ms crossfade. Changing the selected solo band fades out before changing the monitor.
- SOLO is temporary, not automated, not stored in presets, and ends when the editor closes or its band is disabled.
- The Solo monitor is minimum phase even when the EQ uses FIR; it adds no buffering latency, but introduces its own audition-only filter phase.

## Compact window and Solo highlighting

Default editor size: 1000 x 700. Drag the lower-right corner or use Size for preset dimensions. The current instance remembers its window size while loaded. In Solo, only the auditioned band is highlighted with a yellow gradient; other bands and the combined response are dimmed. The floating panel avoids the highlighted area where space permits.

## Global rotary controls

Balance, Mix and Output use rotary controls. Balance displays L/R percentages or M/S percentages with C for centre. Double-click resets Balance/Output to zero and Mix to 100%. Balance and polarity are post-Mix global controls, so they remain effective at 0% Mix; internal bypass restores the unbalanced, non-inverted signal. Output gain retains the existing wet-path behaviour.

## Mid / Side output

Select Mid / Side from the mode dropdown to automatically open independent Mid and Side gain knobs and INV MID / INV SIDE polarity switches. The centre-preserving balance knob favours Mid to the left and Side to the right. INV ALL flips the final stereo signal. M/S controls act after Mix, require stereo, are stored in presets, and are released by bypass. Legacy states initialise the four appended parameters to unity gain and normal polarity.

The Mid / Side panel follows the mode dropdown: visible in stereo Mid / Side mode and hidden in L/R.
