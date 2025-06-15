# Define installer parameters with default values
!ifndef PROJECT_NAME
  !define PROJECT_NAME "MyApp"
!endif

!ifndef PROJECT_VERSION
  !define PROJECT_VERSION "1.0"
!endif

!ifndef PUBLISHER
  !define PUBLISHER "Unknown Company"
!endif

!ifndef DEPLOY_DIR
  !define DEPLOY_DIR "_deploy"
!endif

!define VC_REDIST_URL "https://aka.ms/vs/16/release/vc_redist.x64.exe"
!define VC_REDIST_FILE "vc_redist.x64.exe"

# Name of the installer
OutFile "${PROJECT_NAME}Installer.exe"

# Name of the application
Name "${PROJECT_NAME}"

# Default installation directory
InstallDir "$PROGRAMFILES\${PROJECT_NAME}"

# Request administrator rights
RequestExecutionLevel admin

# Includes
!include "MUI2.nsh"
!include "nsDialogs.nsh"
!include "FileFunc.nsh"
!include "LogicLib.nsh"

# Language selection
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "German"

# Page definitions
!insertmacro MUI_PAGE_WELCOME
#!insertmacro MUI_PAGE_LICENSE "license.txt"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_RUN "$INSTDIR\${PROJECT_NAME}.exe"
!define MUI_FINISHPAGE_RUN_TEXT "Run ${PROJECT_NAME}"
#!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\README.txt"
#!define MUI_FINISHPAGE_SHOWREADME_TEXT "Show README"
!insertmacro MUI_PAGE_FINISH

UninstPage uninstConfirm
UninstPage instfiles

# Function to check if the user has administrator rights
Function VerifyUserIsAdmin
  UserInfo::GetAccountType
  Pop $0
  ${If} $0 != "admin"
    MessageBox MB_ICONSTOP "Administrator rights required!"
    SetErrorLevel 740
    Quit
  ${EndIf}
FunctionEnd

# Function to check and install VC++ Redistributable
Function CheckVCRedist
  ReadRegDWORD $0 HKLM "SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64" "Installed"
  IfErrors InstallVCRedist
  StrCmp $0 1 0 done
InstallVCRedist:
  DetailPrint "Downloading and installing VC++ Redistributable..."
  nsExec::ExecToLog '"$WINDIR\System32\WindowsPowerShell\v1.0\powershell.exe" -Command "Invoke-WebRequest -Uri ${VC_REDIST_URL} -OutFile $TEMP\${VC_REDIST_FILE}"'
  ExecWait '"$TEMP\${VC_REDIST_FILE}" /install /quiet /norestart'
done:
FunctionEnd

# Section definitions
SectionGroup /e "${PROJECT_NAME}" SEC01
  Section "Main Application" SEC01A
    SectionIn RO
    # Set output path to the installation directory
    SetOutPath "$INSTDIR"

    # Copy files to the installation directory
    File /r "${DEPLOY_DIR}\bin\*"

    # Write uninstall information to the registry
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROJECT_NAME}" "DisplayName" "${PROJECT_NAME}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROJECT_NAME}" "UninstallString" "$INSTDIR\uninstall.exe"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROJECT_NAME}" "InstallLocation" "$INSTDIR"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROJECT_NAME}" "DisplayIcon" "$INSTDIR\${PROJECT_NAME}.exe"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROJECT_NAME}" "DisplayVersion" "${PROJECT_VERSION}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROJECT_NAME}" "Publisher" "${PUBLISHER}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROJECT_NAME}" "NoModify" "1"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROJECT_NAME}" "NoRepair" "1"

    # Write uninstaller
    WriteUninstaller "$INSTDIR\uninstall.exe"
  SectionEnd

  Section "Create Start Menu Shortcut" SEC01B
    SectionIn 1
    # Create Start Menu folder
    CreateDirectory "$SMPROGRAMS\${PROJECT_NAME}"
    # Create shortcuts
    CreateShortcut "$SMPROGRAMS\${PROJECT_NAME}\${PROJECT_NAME}.lnk" "$INSTDIR\${PROJECT_NAME}.exe"
    CreateShortcut "$SMPROGRAMS\${PROJECT_NAME}\Uninstall.lnk" "$INSTDIR\uninstall.exe"
  SectionEnd

  Section "Create Desktop Shortcut" SEC01C
    SectionIn 1
    # Create desktop shortcut
    CreateShortcut "$DESKTOP\${PROJECT_NAME}.lnk" "$INSTDIR\${PROJECT_NAME}.exe"
  SectionEnd
SectionGroupEnd

Section "VC++ Redistributable" SEC02
  SectionIn 1
  # Check and install VC++ Redistributable
  Call CheckVCRedist
SectionEnd

# Uninstall section
Section "Uninstall"
  # Remove files and directories
  Delete "$INSTDIR\${PROJECT_NAME}.exe"
  Delete "$INSTDIR\uninstall.exe"
  RMDir /r "$INSTDIR"

  # Remove shortcuts
  Delete "$DESKTOP\${PROJECT_NAME}.lnk"
  Delete "$SMPROGRAMS\${PROJECT_NAME}\Uninstall.lnk"
  Delete "$SMPROGRAMS\${PROJECT_NAME}\${PROJECT_NAME}.lnk"
  RMDir "$SMPROGRAMS\${PROJECT_NAME}"

  # Remove registry entries
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROJECT_NAME}"
SectionEnd

# Function to check if the user has administrator rights
Function .onInit
  Call VerifyUserIsAdmin
  StrCpy $Language ${LANG_ENGLISH}
  !insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd

# Language strings
LangString MUI_INNERTEXT_COMPONENTS_DESCRIPTION_TITLE ${LANG_ENGLISH} "Description"
LangString MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO ${LANG_ENGLISH} "Select the components you want to install."
LangString MUI_TEXT_WELCOME_INFO_TITLE ${LANG_ENGLISH} "Welcome to the ${PROJECT_NAME} Setup Wizard"
LangString MUI_TEXT_WELCOME_INFO_TEXT ${LANG_ENGLISH} "This will install ${PROJECT_NAME} version ${PROJECT_VERSION} on your computer. Click Next to continue."
LangString MUI_TEXT_COMPONENTS_TITLE ${LANG_ENGLISH} "Select Components"
LangString MUI_TEXT_COMPONENTS_SUBTITLE ${LANG_ENGLISH} "Choose which components you want to install."
LangString MUI_TEXT_DIRECTORY_TITLE ${LANG_ENGLISH} "Select Destination Location"
LangString MUI_TEXT_DIRECTORY_SUBTITLE ${LANG_ENGLISH} "Choose a folder to install ${PROJECT_NAME}."
LangString MUI_TEXT_INSTALLING_TITLE ${LANG_ENGLISH} "Installing"
LangString MUI_TEXT_INSTALLING_SUBTITLE ${LANG_ENGLISH} "Please wait while ${PROJECT_NAME} is being installed."
LangString MUI_TEXT_FINISH_TITLE ${LANG_ENGLISH} "Completing the ${PROJECT_NAME} Setup Wizard"
LangString MUI_TEXT_FINISH_SUBTITLE ${LANG_ENGLISH} "Setup has finished installing ${PROJECT_NAME} on your computer."
LangString MUI_TEXT_ABORT_TITLE ${LANG_ENGLISH} "Installation Aborted"
LangString MUI_TEXT_ABORT_SUBTITLE ${LANG_ENGLISH} "The installation was aborted."

LangString MUI_INNERTEXT_COMPONENTS_DESCRIPTION_TITLE ${LANG_GERMAN} "Beschreibung"
LangString MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO ${LANG_GERMAN} "Wählen Sie die Komponenten aus, die Sie installieren möchten."
LangString MUI_TEXT_WELCOME_INFO_TITLE ${LANG_GERMAN} "Willkommen zum ${PROJECT_NAME} Setup-Assistenten"
LangString MUI_TEXT_WELCOME_INFO_TEXT ${LANG_GERMAN} "Dies wird ${PROJECT_NAME} Version ${PROJECT_VERSION} auf Ihrem Computer installieren. Klicken Sie auf Weiter, um fortzufahren."
LangString MUI_TEXT_COMPONENTS_TITLE ${LANG_GERMAN} "Komponenten auswählen"
LangString MUI_TEXT_COMPONENTS_SUBTITLE ${LANG_GERMAN} "Wählen Sie die Komponenten aus, die Sie installieren möchten."
LangString MUI_TEXT_DIRECTORY_TITLE ${LANG_GERMAN} "Zielverzeichnis auswählen"
LangString MUI_TEXT_DIRECTORY_SUBTITLE ${LANG_GERMAN} "Wählen Sie einen Ordner, um ${PROJECT_NAME} zu installieren."
LangString MUI_TEXT_INSTALLING_TITLE ${LANG_GERMAN} "Installation"
LangString MUI_TEXT_INSTALLING_SUBTITLE ${LANG_GERMAN} "Bitte warten Sie, während ${PROJECT_NAME} installiert wird."
LangString MUI_TEXT_FINISH_TITLE ${LANG_GERMAN} "Abschluss des ${PROJECT_NAME} Setup-Assistenten"
LangString MUI_TEXT_FINISH_SUBTITLE ${LANG_GERMAN} "Die Installation von ${PROJECT_NAME} auf Ihrem Computer ist abgeschlossen."
LangString MUI_TEXT_ABORT_TITLE ${LANG_GERMAN} "Installation abgebrochen"
LangString MUI_TEXT_ABORT_SUBTITLE ${LANG_GERMAN} "Die Installation wurde abgebrochen."

LangString MUI_BUTTONTEXT_FINISH ${LANG_ENGLISH} "Finish"
LangString MUI_BUTTONTEXT_FINISH ${LANG_GERMAN} "Fertigstellen"
LangString MUI_TEXT_FINISH_INFO_TITLE ${LANG_ENGLISH} "Completing the ${PROJECT_NAME} Setup Wizard"
LangString MUI_TEXT_FINISH_INFO_TITLE ${LANG_GERMAN} "Abschluss des ${PROJECT_NAME} Setup-Assistenten"
LangString MUI_TEXT_FINISH_INFO_REBOOT ${LANG_ENGLISH} "You must restart your system for the configuration changes made to ${PROJECT_NAME} to take effect. Click Finish to restart your system."
LangString MUI_TEXT_FINISH_INFO_REBOOT ${LANG_GERMAN} "Sie müssen Ihr System neu starten, damit die Konfigurationsänderungen an ${PROJECT_NAME} wirksam werden. Klicken Sie auf Fertigstellen, um Ihr System neu zu starten."
LangString MUI_TEXT_FINISH_REBOOTNOW ${LANG_ENGLISH} "Restart now"
LangString MUI_TEXT_FINISH_REBOOTNOW ${LANG_GERMAN} "Jetzt neu starten"
LangString MUI_TEXT_FINISH_REBOOTLATER ${LANG_ENGLISH} "Restart later"
LangString MUI_TEXT_FINISH_REBOOTLATER ${LANG_GERMAN} "Später neu starten"
LangString MUI_TEXT_FINISH_INFO_TEXT ${LANG_ENGLISH} "Setup has finished installing ${PROJECT_NAME} on your computer."
LangString MUI_TEXT_FINISH_INFO_TEXT ${LANG_GERMAN} "Die Installation von ${PROJECT_NAME} auf Ihrem Computer ist abgeschlossen."
