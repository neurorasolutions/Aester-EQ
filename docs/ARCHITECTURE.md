# Architecture

The processor manages 24 parameterized bands. Static IIR sections and time-varying gain sections cover minimum-phase processing. Dynamic detectors measure band-limited or independently filtered key energy. The Spectral engine uses a 1024-sample STFT with 256-sample hop, square-root Hann windows and overlap/add reconstruction.

FIR design is performed off the audio thread. Analog FIR represents the complex response; Linear FIR uses symmetric magnitude-derived kernels. A stereo matrix supports Left/Right and Mid/Side placement. Audio-thread ownership transfers avoid kernel deletion in the callback.

The global Mix parameter is appended to the parameter list (396 total, including four appended Mid/Side output parameters). A 20 ms SmoothedValue multiplies the bypass wet coefficient. Both the IIR/FIR path and Spectral engine already carry an aligned dry path. This avoids adding a second delay or allocating in the audio callback. Balance, polarity and Mid/Side output controls act after Mix and are blended toward neutral by bypass.

The editor combines a resizable response graph, a floating selected-band panel and rotary global controls. The Mid/Side panel follows the output-mode dropdown. Parameter attachments connect these controls to the processor. Freeze suppresses spectrum-display updates, not audio processing or metering. Parameter attachments remain the source of control values.

Source state schema 8 appends independent Mid/Side gain and polarity controls. Earlier states load these at unity gain and normal polarity; Mix defaults to 100% when missing. Product identifiers are separate from other DropDownAcademy plug-ins.

Solo is a non-persistent atomic monitoring request. A smoothed TPT state-variable filter isolates a region of the final EQ output, with L/R/M/S routing and a 20 ms crossfade. It introduces filter phase but no buffering latency and allocates no memory in the audio callback. Closing the editor clears the request.
