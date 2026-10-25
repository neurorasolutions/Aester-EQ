# Installing Aester EQ

Close your DAW and save sessions before installation.

## macOS Apple Silicon

Download `Aester-EQ-DropDownAcademy-1.3.0-macOS-arm64.dmg`, open it and run `Aester-EQ-DropDownAcademy-Installer.pkg`. The package installs:

- `Aester EQ.component` in `/Library/Audio/Plug-Ins/Components` (Audio Unit)
- `Aester EQ.app` in `/Applications` (standalone)

Restart the DAW and look for **DropDownAcademy / Aester EQ**. The package is ad hoc signed and not notarized: if macOS refuses the first launch of the standalone, right-click the app, choose **Open** and confirm. No kernel extension, driver or background service is installed.

## Windows x64

Run `Aester-EQ-DropDownAcademy-1.3.0-Windows-x64.exe` and allow the administrator prompt. The installer places:

- `Aester EQ.vst3` in `C:\Program Files\Common Files\VST3\Aester EQ.vst3` (VST3)
- the standalone in `C:\Program Files\DropDownAcademy\Aester EQ`

Restart the DAW and rescan plug-ins. The installer is unsigned, so Windows SmartScreen may ask for confirmation. If the host reports missing runtime DLLs, install the official Microsoft Visual C++ 2015-2022 x64 Redistributable, never individual DLL files from third-party download sites.

## Updating from a 1.2.0 build

The 1.3.0 packages use a new bundle identifier and manufacturer code. A 1.2.0 installation is not overwritten: remove the old `Aester EQ.component` / `Aester EQ.vst3` and the old application before installing, then rescan. Sessions saved with 1.2.0 open in 1.3.0 with the same parameters, but hosts that store plug-in IDs may report the previous instance as missing.

## Uninstalling

- macOS: delete `Aester EQ.component` from `/Library/Audio/Plug-Ins/Components` and `Aester EQ.app` from `/Applications`.
- Windows: use **Apps and features** (the uninstaller removes the VST3, the standalone and the Start menu shortcuts).

User presets and session files are not deleted by the uninstallers.

## Separate identity

Aester EQ has its own product name, bundle identifier, plug-in code and installer receipt. It does not overwrite other DropDownAcademy products. AAX development binaries are not included in these installers.
