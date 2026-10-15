# Architecture

The processor manages 24 parameterized bands. Static IIR sections and time-varying gain sections cover minimum-phase processing. Dynamic detectors measure band-limited or independently filtered key energy. The Spectral engine uses a 1024-sample STFT with 256-sample hop, square-root Hann windows and overlap/add reconstruction.

FIR design is performed off the audio thread. Analog FIR represents the complex response; Linear FIR uses symmetric magnitude-derived kernels. A stereo matrix supports Left/Right and Mid/Side placement. Audio-thread ownership transfers avoid kernel deletion in the callback.

The global Mix parameter is appended to the parameter list (392 total). A 20 ms SmoothedValue multiplies the bypass wet coefficient. Both the IIR/FIR path and Spectral engine already carry an aligned dry path. This avoids adding a second delay or allocating in the audio callback. Balance and polarity are blended toward neutral with the effective wet coefficient.

The editor separates the response graph, paged band-card strip and persistent output rail. Each band card has independent parameter attachments for frequency, gain, Q and enabled state. Freeze suppresses spectrum-display updates, not audio processing or metering. Parameter attachments remain the source of control values.

Source state schema 7 appends Mix with a 100% default when loading earlier state. Product identifiers are separate from other Neurora plug-ins.
