# Installing Aester EQ

Close your DAW and save sessions before installation.

## macOS Apple Silicon

Open the DMG and run `Aester-EQ-Neurora-Installer.pkg`. It installs `Aester EQ.component` in `/Library/Audio/Plug-Ins/Components` and `Aester EQ.app` in `/Applications`. Restart the DAW and look for Neurora / Aester EQ. Distribution signing and notarization are not configured.

## Windows x64

Run `Aester-EQ-Neurora-1.1.0-Windows-x64.exe`. VST3 installs in `C:\Program Files\Common Files\VST3\Aester EQ.vst3`; standalone defaults to `C:\Program Files\Neurora\Aester EQ`. Restart and rescan the DAW. The unsigned installer needs administrator permission. If runtime DLLs are missing, use the official Microsoft Visual C++ x64 Redistributable, not individual DLL download sites.

Windows interactive installation/DAW testing must be distinguished from build/CTest results. Use the Apps uninstaller to remove the package. User presets and sessions are not intentionally removed.

## Separate identity

Aester EQ has its own product name, bundle identifier, plug-in code and installer receipt. It does not overwrite other products. AAX development binaries are not included in these installers.
