Unicode true
!include "MUI2.nsh"
!include "x64.nsh"
Name "Aester EQ by DropDownAcademy"
OutFile "${OUTPUT}"
InstallDir "$PROGRAMFILES64\DropDownAcademy\Aester EQ"
RequestExecutionLevel admin
!define MUI_ABORTWARNING
!define MUI_WELCOMEPAGE_TITLE "Aester EQ by DropDownAcademy"
!define MUI_WELCOMEPAGE_TEXT "Free educational equalizer by DropDownAcademy.$\r$\n$\r$\nPrecision static, dynamic and spectral equalization with paged band cards, latency-aligned Mix and analyzer Freeze.$\r$\n$\r$\nhttps://www.dropdownacademy.com$\r$\n$\r$\nClose your audio applications before installing."
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "${SOURCE}\LICENSE"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"
Function .onInit
  ${IfNot} ${RunningX64}
    MessageBox MB_ICONSTOP "This installer requires 64-bit Windows."
    Abort
  ${EndIf}
  SetRegView 64
FunctionEnd
Section "Aester EQ"
  SetOutPath "$COMMONFILES64\VST3\Aester EQ.vst3"
  File /r "${BUILD}\DistillazioneEQ_artefacts\Release\VST3\Aester EQ.vst3\*"
  SetOutPath "$INSTDIR"
  File "${BUILD}\DistillazioneEQ_artefacts\Release\Standalone\Aester EQ.exe"
  File "${SOURCE}\LICENSE"
  File "${SOURCE}\THIRD_PARTY_NOTICES.md"
  SetOutPath "$INSTDIR\docs"
  File "${SOURCE}\docs\USER_GUIDE.md"
  File "${SOURCE}\docs\PROVENANCE.md"
  File /nonfatal "${SOURCE}\MANUALS\*.pdf"
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  CreateDirectory "$SMPROGRAMS\DropDownAcademy"
  CreateShortcut "$SMPROGRAMS\DropDownAcademy\Aester EQ.lnk" "$INSTDIR\Aester EQ.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DropDownAcademyAesterEQ" "DisplayName" "Aester EQ by DropDownAcademy"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DropDownAcademyAesterEQ" "DisplayVersion" "${VERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DropDownAcademyAesterEQ" "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DropDownAcademyAesterEQ" "Publisher" "DropDownAcademy"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DropDownAcademyAesterEQ" "URLInfoAbout" "https://www.dropdownacademy.com"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DropDownAcademyAesterEQ" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DropDownAcademyAesterEQ" "NoRepair" 1
SectionEnd
Section "Uninstall"
  SetRegView 64
  Delete "$COMMONFILES64\VST3\Aester EQ.vst3\Contents\x86_64-win\Aester EQ.vst3"
  Delete "$COMMONFILES64\VST3\Aester EQ.vst3\Contents\Resources\moduleinfo.json"
  RMDir "$COMMONFILES64\VST3\Aester EQ.vst3\Contents\x86_64-win"
  RMDir "$COMMONFILES64\VST3\Aester EQ.vst3\Contents\Resources"
  RMDir "$COMMONFILES64\VST3\Aester EQ.vst3\Contents"
  RMDir "$COMMONFILES64\VST3\Aester EQ.vst3"
  Delete "$INSTDIR\Aester EQ.exe"
  Delete "$INSTDIR\LICENSE"
  Delete "$INSTDIR\THIRD_PARTY_NOTICES.md"
  Delete "$INSTDIR\docs\USER_GUIDE.md"
  Delete "$INSTDIR\docs\PROVENANCE.md"
  Delete "$INSTDIR\docs\*.pdf"
  RMDir "$INSTDIR\docs"
  Delete "$INSTDIR\Uninstall.exe"
  RMDir "$INSTDIR"
  Delete "$SMPROGRAMS\DropDownAcademy\Aester EQ.lnk"
  RMDir "$SMPROGRAMS\DropDownAcademy"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DropDownAcademyAesterEQ"
SectionEnd
