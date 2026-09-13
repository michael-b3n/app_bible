;;
;; NOTE!
;; This script is used as template and will be copied by CMake to the binary folder
;; all variables with ${SOME_NAME} will be replaced by CMake during copy
;;

#define common_file_dir "${INNO_SETUP_REPO_BASE_PATH}/setup"

#define AppLongName     "${INNO_SETUP_APP_LONG_NAME}"
#define AppExeName      "${INNO_SETUP_APP_EXE_NAME}"
#define MyAppVersion    "${INNO_SETUP_APP_VERSION_NUMBER}"
#define MyAppPublisher  "${INNO_SETUP_APP_PUBLISHER}"
#define CompanyName     "${INNO_SETUP_APP_COMANY_NAME}"
#define MyAppURL        "${INNO_SETUP_APP_URL}"
#define ARCH            "x86_64"

[InstallDelete]
Type: filesandordirs; Name: "{app}"

[Setup]
AppId={{54A4C114-E258-4E27-885A-13D3E9EBF609}
AppName={#AppLongName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}/${INNO_SETUP_APP_INSTALL_FOLDER}
DefaultGroupName=${INNO_SETUP_APP_INSTALL_FOLDER}
OutputDir=.
OutputBaseFilename=${INNO_SETUP_OUTPUT_NAME}
SetupIconFile=${INNO_SETUP_OUTPUT_ICON}
UninstallDisplayIcon={app}\bin\{#AppExeName}
UninstallDisplayName={#AppLongName}
Compression=lzma
SolidCompression=yes
ArchitecturesAllowed = x64compatible
ArchitecturesInstallIn64BitMode = x64compatible
UsePreviousAppDir=Yes
PrivilegesRequiredOverridesAllowed=dialog

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "autostart"; Description: "Start {#AppLongName} automatically when signing in"; GroupDescription: "Startup:"
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "bin\*"; DestDir: "{app}\bin"; Flags: ignoreversion recursesubdirs
Source: "share\*"; DestDir: "{app}\share"; Flags: ignoreversion recursesubdirs

[Icons]
Name: "{group}\{#AppLongName}"; Filename: "{app}\bin\{#AppExeName}"
Name: "{commondesktop}\{#AppLongName}"; Filename: "{app}\bin\{#AppExeName}"; Tasks: desktopicon
Name: "{commonappdata}\Microsoft\Internet Explorer\Quick Launch\{#AppLongName}"; Filename: "{app}\bin\{#AppExeName}"; Tasks: quicklaunchicon
; The startup folder of all users for an all users install, of the current user otherwise.
Name: "{autostartup}\{#AppLongName}"; Filename: "{app}\bin\{#AppExeName}"; Tasks: autostart

[Run]
Filename: "{app}\bin\{#AppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(AppLongName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[Code]
// Releases up to v1.5 were installed as "Bible Assistant" without an explicit AppId.
const
  LegacyUninstallKey = 'Software\Microsoft\Windows\CurrentVersion\Uninstall\Bible Assistant_is1';
  LegacyExeName = 'app_bible_assistant.exe';

// The install mode dialog allowed both all users and current user installs.
function FindLegacyUninstaller(var Uninstaller: String): Boolean;
begin
  Result := RegQueryStringValue(HKLM64, LegacyUninstallKey, 'UninstallString', Uninstaller)
         or RegQueryStringValue(HKCU, LegacyUninstallKey, 'UninstallString', Uninstaller);
end;

function PrepareToInstall(var NeedsRestart: Boolean): String;
var
  Uninstaller: String;
  ResultCode: Integer;
begin
  Result := '';
  if not FindLegacyUninstaller(Uninstaller) then
    exit;

  // A running tray app locks its files, it has no channel to be asked to quit.
  Exec(ExpandConstant('{sys}\taskkill.exe'), '/F /IM ' + LegacyExeName, '', SW_HIDE, ewWaitUntilTerminated, ResultCode);

  // ShellExec, an all users install needs elevation to be removed.
  if not ShellExec('', RemoveQuotes(Uninstaller), '/VERYSILENT /SUPPRESSMSGBOXES /NORESTART', '', SW_HIDE, ewWaitUntilTerminated, ResultCode)
     or (ResultCode <> 0) then
    Result := 'The previous version "Bible Assistant" could not be removed. Please uninstall it manually and run setup again.';
end;
