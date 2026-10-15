# Contributing to Aester EQ

Contributions are welcome under AGPL-3.0-only. Keep third-party attribution and source notices intact.

Report the operating system, CPU architecture, DAW/version, sample rate, buffer size, processing mode and steps to reproduce. Include a small preset or synthetic input where possible. Avoid private session data.

Build and run all seven CTest suites before proposing DSP changes. Document parameter ordering and state migration when adding controls. Attach screenshots at minimum/default/large editor sizes for interface changes. Distinguish a shared processor test from an actual AU/VST3 host test.

Use deterministic signal measurements to support claims about gain, latency or noise. Listening impressions should state their input, settings and limitations. Do not present unmeasured quality claims as facts.
