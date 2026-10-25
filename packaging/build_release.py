#!/usr/bin/env python3
"""Build a DropDownAcademy-branded PKG/DMG without installing it."""
import argparse, hashlib, pathlib, plistlib, shutil, subprocess, tarfile, tempfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
BRAND = "DropDownAcademy"
PRODUCT = "Aester EQ"
IDENTIFIER = "com.dropdownacademy.aestereq"


def run(*args):
    subprocess.run([str(x) for x in args], check=True)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--build-dir", type=pathlib.Path, default=ROOT / "build")
    args = parser.parse_args()
    build = args.build_dir.resolve()
    release = ROOT / "releases"
    release.mkdir(exist_ok=True)
    artifacts = build / "DistillazioneEQ_artefacts/Release"
    au = artifacts / "AU/Aester EQ.component"
    app = artifacts / "Standalone/Aester EQ.app"
    if not au.exists() or not app.exists():
        raise SystemExit("Build the AU and standalone before packaging.")
    version = plistlib.loads((au / "Contents/Info.plist").read_bytes())["CFBundleShortVersionString"]

    run("codesign", "--force", "--deep", "--sign", "-", au)
    run("codesign", "--force", "--deep", "--sign", "-", app)

    source = release / f"Aester-EQ-{BRAND}-{version}-source.tar.gz"
    with tarfile.open(source, "w:gz") as archive:
        for path in sorted(ROOT.iterdir()):
            if path.name in {"releases", ".git", ".DS_Store", ".packaging-build"} or path.name.startswith("build"):
                continue

            def filter_entry(info):
                if any(part in {".DS_Store", "__pycache__", ".git"} for part in pathlib.PurePosixPath(info.name).parts):
                    return None
                return info

            archive.add(path, arcname="Aester-EQ/" + path.name, filter=filter_entry)

    with tempfile.TemporaryDirectory(prefix="aester-package-") as work:
        work = pathlib.Path(work)
        payload = work / "payload"
        (payload / "Library/Audio/Plug-Ins/Components").mkdir(parents=True)
        (payload / "Applications").mkdir()
        shutil.copytree(au, payload / "Library/Audio/Plug-Ins/Components/Aester EQ.component")
        shutil.copytree(app, payload / "Applications/Aester EQ.app")

        components = work / "components.plist"
        run("pkgbuild", "--analyze", "--root", payload, components)
        rows = plistlib.loads(components.read_bytes())
        for row in rows:
            row["BundleIsRelocatable"] = False
        components.write_bytes(plistlib.dumps(rows))

        component = work / "payload.pkg"
        run("pkgbuild", "--root", payload, "--component-plist", components,
            "--scripts", ROOT / "packaging/scripts",
            "--identifier", f"{IDENTIFIER}.installer",
            "--version", version, "--install-location", "/", component)

        distribution = work / "Distribution.xml"
        distribution.write_text(f'''<?xml version="1.0" encoding="utf-8"?>
<installer-gui-script minSpecVersion="2">
<title>Aester EQ by {BRAND} — Educational Equalizer</title>
<options customize="never" hostArchitectures="arm64"/>
<domains enable_anywhere="false" enable_currentUserHome="false" enable_localSystem="true"/>
<allowed-os-versions><os-version min="11.0"/></allowed-os-versions>
<welcome file="Welcome.html" mime-type="text/html"/>
<readme file="ReadMe.html" mime-type="text/html"/>
<license file="License.txt" mime-type="text/plain"/>
<choices-outline><line choice="aester"/></choices-outline>
<choice id="aester" visible="false"><pkg-ref id="{IDENTIFIER}.installer"/></choice>
<pkg-ref id="{IDENTIFIER}.installer" version="{version}" onConclusion="none">payload.pkg</pkg-ref>
</installer-gui-script>''')

        resources = work / "resources"
        shutil.copytree(ROOT / "packaging/resources", resources)
        shutil.copy2(ROOT / "LICENSE", resources / "License.txt")
        shutil.copy2(ROOT / "assets/dropdownacademy-logo.png", resources / "dropdownacademy-logo.png")

        image = work / "image"
        image.mkdir()
        pkg = image / f"Aester-EQ-{BRAND}-Installer.pkg"
        run("productbuild", "--distribution", distribution, "--resources", resources,
            "--package-path", work, pkg)

        shutil.copy2(source, image / source.name)
        readme = (ROOT / "README.md").read_text()
        readme = readme.replace(f"(releases/Aester-EQ-{BRAND}-{version}-macOS-arm64.dmg)",
                                f"(Aester-EQ-{BRAND}-Installer.pkg)")
        readme = readme.replace(
            f"(releases/Aester-EQ-{BRAND}-{version}-Windows-x64.exe)",
            f"(https://github.com/neurorasolutions/Aester-EQ/releases/tag/aester-v{version})")
        (image / "README.md").write_text(readme)
        shutil.copy2(ROOT / "LICENSE", image / "LICENSE")
        shutil.copytree(ROOT / "docs", image / "docs")
        for document in ["THIRD_PARTY_NOTICES.md", "CONTRIBUTING.md", "CHANGELOG.md"]:
            shutil.copy2(ROOT / document, image / document)
        shutil.copytree(ROOT / "assets", image / "assets")
        if (ROOT / "MANUALS").exists():
            shutil.copytree(ROOT / "MANUALS", image / "MANUALS")
        shutil.copy2(ROOT / "packaging/resources/ReadMe.html", image / "START-HERE.html")

        dmg = release / f"Aester-EQ-{BRAND}-{version}-macOS-arm64.dmg"
        run("hdiutil", "create", "-volname", f"Aester EQ by {BRAND}", "-srcfolder", image,
            "-format", "UDZO", "-ov", dmg)
        run("hdiutil", "verify", dmg)
        shutil.copy2(pkg, release / f"Aester-EQ-{BRAND}-Installer.pkg")

    sums = []
    for path in sorted(release.iterdir()):
        if path.is_file() and path.name != "SHA256SUMS":
            sums.append(hashlib.sha256(path.read_bytes()).hexdigest() + "  " + path.name)
    (release / "SHA256SUMS").write_text("\n".join(sums) + "\n")
    print("Release ready:", dmg)


if __name__ == "__main__":
    main()
