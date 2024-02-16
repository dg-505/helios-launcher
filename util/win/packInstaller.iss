#define Name "helios-launcher"
#define Version "0.1.0"
#define ExeName "helios-launcher.exe"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{CAE71730-D38E-4F3F-B71A-B52DCF4594F4}
AppName={#Name}
AppVersion={#Version}
DefaultDirName={localappdata}\Programs\{#Name}
DefaultGroupName={#Name}
AllowNoIcons=yes
LicenseFile=../../../release/WindowsInstaller/LICENSE
OutputDir=../../../release
OutputBaseFilename=helios-launcher-v{#Version}-win64-setup
SetupIconFile=../../../release/WindowsInstaller/ico/heliospp.ico
Compression=lzma
SolidCompression=yes
UninstallDisplayIcon={app}/ico/heliospp.ico
WizardStyle=modern
AllowCancelDuringInstall=False
DisableDirPage=no
AppCopyright=GPL v3
UsePreviousPrivileges=False
PrivilegesRequired=lowest
FlatComponentsList=False
DisableWelcomePage=False
AlwaysShowGroupOnReadyPage=True
AlwaysShowDirOnReadyPage=True

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

; remove entire program directory when installing or uninstalling
[UninstallDelete]
Type: filesandordirs; Name: "{app}"

[InstallDelete]
Type: filesandordirs; Name: "{app}"

[Files]
Source: "../../../release/WindowsInstaller/*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs;

[Icons]
Name: "{group}\HELIOS++ launcher"; Filename: "{app}\{#ExeName}"
Name: "{group}\{cm:UninstallProgram,{#Name}}"; Filename: "{uninstallexe}"; IconFilename: "{app}/ico/heliospp.ico"
Name: "{userdesktop}\{#Name}"; Filename: "{app}\{#ExeName}"; IconFilename: "{app}/ico/heliospp.ico"; Tasks: desktopicon

[Run]
Filename: "{app}\{#ExeName}"; Description: "{cm:LaunchProgram,{#StringChange(Name, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
