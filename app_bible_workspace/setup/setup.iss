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

[Setup]
AppName={#AppLongName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={commonpf}/${INNO_SETUP_APP_INSTALL_FOLDER}
DefaultGroupName=${INNO_SETUP_APP_INSTALL_FOLDER}
OutputDir=.
OutputBaseFilename=${INNO_SETUP_OUTPUT_NAME}
SetupIconFile=${INNO_SETUP_OUTPUT_ICON}
UninstallDisplayIcon={app}\{#AppExeName}
UninstallDisplayName={#AppLongName}
Compression=lzma
SolidCompression=yes
ArchitecturesAllowed = x64
ArchitecturesInstallIn64BitMode = x64
UsePreviousAppDir=Yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 0,6.1

[Files]
;;
;; Executables
;;
Source: "*.exe"; Excludes: "${INNO_SETUP_OUTPUT_NAME}.exe"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs

;;
;; libs
;;
Source: "*.dll"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs

;;
;; Config files
;;
Source: "*.traineddata"; DestDir: "{app}\tesseract"; Flags: ignoreversion recursesubdirs
;Source: "*.par";                  DestDir: "{app}"; Flags: ignoreversion
;Source: "*.stagex";               DestDir: "{app}"; Flags: ignoreversion
;Source: "*.chart";                DestDir: "{app}"; Flags: ignoreversion
;Source: "*.txt";                  DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\{#AppLongName}"; Filename: "{app}\{#AppExeName}"
Name: "{commondesktop}\{#AppLongName}"; Filename: "{app}\{#AppExeName}"; Tasks: desktopicon
Name: "{commonappdata}\Microsoft\Internet Explorer\Quick Launch\{#AppLongName}"; Filename: "{app}\{#AppExeName}"; Tasks: quicklaunchicon

[Run]
Filename: "{app}\{#AppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(AppLongName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
