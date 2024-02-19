#define Name "helios-launcher"
#define Alias "HELIOS++ launcher"
#define Version "0.1.0"
#define ExeName "helios-launcher.exe"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{CAE71730-D38E-4F3F-B71A-B52DCF4594F4}
AppName={#Alias}
AppVersion={#Version}
DefaultDirName={localappdata}\Programs\{#Name}
DefaultGroupName=HELIOS++
AllowNoIcons=yes
LicenseFile=../../../deploy/WindowsInstaller/LICENSE
OutputDir=../../../deploy
OutputBaseFilename={#Name}-v{#Version}-win64-setup
SetupIconFile=../../../deploy/WindowsInstaller/ico/heliospp.ico
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
WizardImageFile=heliospp.bmp
WizardSmallImageFile=heliospp_small.bmp
WizardImageStretch=True

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"
Name: "removeconfig"; Description: "If existent, remove configuration file from previous installation"; GroupDescription: "Reset configuration"; Flags: unchecked

[UninstallDelete]
; remove entire program directory when installing or uninstalling
Type: filesandordirs; Name: "{app}"

[InstallDelete]
Type: filesandordirs; Name: "{app}"
Type: filesandordirs; Name: "{userappdata}\heliospp"; Tasks: removeconfig

[Files]
Source: "../../../deploy/WindowsInstaller/*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs;

[Icons]
Name: "{group}\{#Alias}"; Filename: "{app}/bin/{#ExeName}"
Name: "{group}\{cm:UninstallProgram,{#Alias}}"; Filename: "{uninstallexe}"; IconFilename: "{app}/ico/heliospp.ico"
Name: "{userdesktop}\{#Alias}"; Filename: "{app}\bin\{#ExeName}"; IconFilename: "{app}/ico/heliospp.ico"; Tasks: desktopicon

[Run]
Filename: "{app}\bin\{#ExeName}"; Flags: nowait postinstall skipifsilent; Description: "{cm:LaunchProgram,{#StringChange(Alias, '&', '&&')}}"

[Code]
// check if a configuration file  aleady exists on the file system
function CfgExists(): Boolean;
begin
    Result := FileExists(ExpandConstant('{userappdata}/heliospp/helios-launcher.ini'));
end;

// show configuration file in Windows Explorer
procedure ExplorerShowCfg(Sender: TObject);
var
    ErrorCode: Integer;
begin
    if CfgExists then
    begin
        ShellExec('', ExpandConstant('{userappdata}/heliospp'), '', '', SW_SHOWNORMAL, ewNoWait, ErrorCode);
    end;
end;

// add page to Uninstaller to optionally remove configuration file
var
    ResetCfgUninstallPage: TNewNotebookPage;
    UninstallNextButton: TNewButton;
    ExplorerShowCfgButton: TNewButton;
    ResetCfgCheckbox: TNewCheckBox;
    CfgNotFoundLabel: TNewStaticText;
procedure InitializeUninstallProgressForm();
begin
    ResetCfgUninstallPage := TNewNotebookPage.Create(UninstallProgressForm);
    ResetCfgUninstallPage.Notebook := UninstallProgressForm.InnerNotebook;
    ResetCfgUninstallPage.Parent := UninstallProgressForm.InnerNotebook;
    ResetCfgUninstallPage.Align := alClient

    ResetCfgCheckbox := TNewCheckBox.Create(UninstallProgressForm);
    ResetCfgCheckbox.Parent := ResetCfgUninstallPage;
    ResetCfgCheckbox.Caption := '&Remove configuration file';
    ResetCfgCheckbox.Left := ScaleX(10);
    ResetCfgCheckbox.Top := ScaleY(10);
    ResetCfgCheckbox.Checked := False;
    ResetCfgCheckbox.Width := UninstallProgressForm.InnerNotebook.Width / 3;
    ResetCfgCheckbox.Height := ScaleY(20);
    ResetCfgCheckbox.Enabled := CfgExists;

    ExplorerShowCfgButton := TNewButton.Create(UninstallProgressForm);
    ExplorerShowCfgButton.Parent := ResetCfgUninstallPage;
    ExplorerShowCfgButton.Caption := '&Show configuration file in Windows Explorer';
    ExplorerShowCfgButton.Top := ResetCfgCheckbox.Top;
    ExplorerShowCfgButton.Left := ResetCfgCheckbox.Left + ResetCfgCheckbox.Width;
    ExplorerShowCfgButton.Width := UninstallProgressForm.InnerNotebook.Width - ResetCfgCheckbox.Width - ScaleX(50);
    ExplorerShowCfgButton.Height := ResetCfgCheckbox.Height;
    ExplorerShowCfgButton.OnClick := @ExplorerShowCfg;
    ExplorerShowCfgButton.Enabled := CfgExists;

    CfgNotFoundLabel := TNewStaticText.Create(UninstallProgressForm);
    CfgNotFoundLabel.Parent := ResetCfgUninstallPage;
    CfgNotFoundLabel.Top := ResetCfgCheckbox.Top + ResetCfgCheckbox.Height + ScaleY(10);
    CfgNotFoundLabel.Left := ResetCfgCheckbox.Left;
    CfgNotFoundLabel.Width := UninstallProgressForm.InnerNotebook.Width;
    CfgNotFoundLabel.Height := CfgNotFoundLabel.AdjustHeight();
    CfgNotFoundLabel.WordWrap := True;
    CfgNotFoundLabel.Font.Color := clRed;
    CfgNotFoundLabel.Caption := 'No configuration file found' + #13#10 + '(Default location: ' + ExpandConstant('{userappdata}/heliospp/helios-launcher.ini') + ')';
    CfgNotFoundLabel.Visible := not CfgExists;

    UninstallProgressForm.InnerNotebook.ActivePage := ResetCfgUninstallPage;

    UninstallNextButton := TNewButton.Create(UninstallProgressForm);
    UninstallNextButton.Caption := 'Uninstall';
    UninstallNextButton.Parent := UninstallProgressForm;
    UninstallNextButton.Left := UninstallProgressForm.CancelButton.Left - UninstallProgressForm.CancelButton.Width - ScaleX(10);
    UninstallNextButton.Top := UninstallProgressForm.CancelButton.Top;
    UninstallNextButton.Width := UninstallProgressForm.CancelButton.Width;
    UninstallNextButton.Height := UninstallProgressForm.CancelButton.Height;
    UninstallNextButton.ModalResult := mrOk;
    UninstallNextButton.Default := True;

    UninstallProgressForm.CancelButton.Enabled := True;
    UninstallProgressForm.CancelButton.ModalResult := mrCancel;

    if UninstallProgressForm.ShowModal = mrCancel then Abort;

    UninstallProgressForm.InnerNotebook.ActivePage := UninstallProgressForm.InstallingPage;
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
    if CurUninstallStep = usUninstall then
    begin
         if ResetCfgCheckbox.Checked = True then
         begin
                 DelTree(ExpandConstant('{userappdata}/heliospp'), True, True, True);
         end;
    end;
end;
