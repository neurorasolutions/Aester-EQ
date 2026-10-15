# Optional AAX Native development

The JUCE/Avid SDK AAX Native target remains opt-in using `-DAESTER_BUILD_AAX=ON`; an external SDK may be supplied with `AESTER_AAX_SDK_PATH`. The bundled SDK offers a GPLv3 option; preserve its license notices.

AAX is not included in the product installers. No PACE signing tools/certificate or Pro Tools verification environment is configured. Unsigned AAX development binaries will not load in production Pro Tools. AAX DSP/HDX is not implemented.

See the official Avid/PACE onboarding instructions before enabling a production distribution path. Do not describe a successful shared processor test as an AAX host test.
