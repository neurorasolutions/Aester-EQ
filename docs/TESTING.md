# Verification procedure

Run CTest in a Release build. Seven suites cover polynomial response, parameter interpretation, root sections, digital sections, filter prototypes, static EQ, and the shared audio processor/editor.

Processor checks cover empty startup, mono/stereo routing, state round-trip, bypass, smoothing/automation, Dynamic/Spectral selectivity, external key behavior, stereo/anti-phase analyzer input, FIR resolution/latency, delayed bypass, peak metering, editor Undo/Redo and allocation instrumentation.

Aester adds 18 Mix impulse cases: three phase modes × Static/Spectral × 0/50/100% Mix. With +6 dB wet output, expected peak is `0.25 * (1 + mix * (10^(6/20)-1))` at the reported latency. Maximum sample error must stay below 0.0002. This checks dry alignment and arithmetic, not every possible changing-kernel transient.

The characterization executable measures only Aester, using synthetic sine input at eleven frequencies across five configurations. It reports amplitude, peak and latency without loading any other plug-in. The plots connect discrete measured points and are not continuous sweeps.

GUI screenshots are synthetic fixtures rendered by the actual editor. Check default/minimum/large layouts and all band pages, empty/static/dynamic states, Freeze/Resume and the output panel. Automated checks are not a substitute for interactive DAW installation, parameter automation or accessibility testing.

Report limitations and failures without suppressing them. Callback allocation instrumentation observes C++ allocation operators; it does not prove all operating-system or third-party allocation paths are absent.

