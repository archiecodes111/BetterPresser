[Setup]
AppName=BetterPresser
AppVersion=1.0.0
AppPublisher=Archishman
AppPublisherURL=https://github.com/archiecodes111/BetterPresser
DefaultDirName={autopf}\BetterPresser
DefaultGroupName=BetterPresser
OutputDir=installer_build
OutputBaseFilename=BetterPresser_Windows_Installer
Compression=lzma
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64
SetupIconFile=compiler:SetupClassicIcon.ico
WizardStyle=modern
UninstallDisplayIcon={app}\BetterPresser.exe

[Tasks]
Name: "vst3"; Description: "VST3 Plugin (64-bit)"; GroupDescription: "Components to Install:"; Flags: checkedonce
Name: "standalone"; Description: "Standalone Application"; GroupDescription: "Components to Install:"; Flags: checkedonce
Name: "desktopicon"; Description: "Create a desktop shortcut (Standalone)"; GroupDescription: "Additional icons:"; Flags: unchecked

[Files]
; Standalone App
Source: "build\BetterPresser_artefacts\Release\Standalone\BetterPresser.exe"; DestDir: "{app}"; Tasks: standalone; Flags: ignoreversion
; VST3 Bundle
Source: "build\BetterPresser_artefacts\Release\VST3\BetterPresser.vst3\*"; DestDir: "{commoncf64}\VST3\BetterPresser.vst3"; Tasks: vst3; Flags: ignoreversion recursesubdirs createallsubdirs
; Documentation
Source: "docs\User_Guide.pdf"; DestDir: "{app}\Docs"; Flags: ignoreversion
Source: "docs\Technical_Summary.pdf"; DestDir: "{app}\Docs"; Flags: ignoreversion
Source: "TEST_REPORT.md"; DestDir: "{app}\Docs"; Flags: ignoreversion skipifsourcedoesntexist
Source: "RELEASE_NOTES.md"; DestDir: "{app}\Docs"; Flags: ignoreversion skipifsourcedoesntexist

[Icons]
Name: "{group}\BetterPresser"; Filename: "{app}\BetterPresser.exe"; Tasks: standalone
Name: "{group}\User Guide"; Filename: "{app}\Docs\User_Guide.pdf"
Name: "{group}\Uninstall BetterPresser"; Filename: "{uninstallexe}"
Name: "{autodesktop}\BetterPresser"; Filename: "{app}\BetterPresser.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\BetterPresser.exe"; Description: "Launch BetterPresser Standalone"; Flags: nowait postinstall skipifsilent; Tasks: standalone
