; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!
 #define MyAppName "EmulatorTM"  
#define MyAppExeName "EmulatorTM.exe"
[Setup]
AppVersion=1.0.0
VersionInfoVersion=1.0.0
VersionInfoTextVersion=1.0.0

AppName=����������
AppVerName=����������̻
DefaultGroupName=����������

AppPublisher=��� ����������������, ��� 103
VersionInfoCompany=��� ����������������

AppPublisherURL=http://www.elmeh.ru
AppSupportURL=http://www.elmeh.ru
AppUpdatesURL=http://www.elmeh.ru

DefaultDirName={pf32}\EmulatorTM\
OutputDir=.\
OutputBaseFilename=SETUP
Compression=lzma
SolidCompression=true
DisableDirPage=true
DisableReadyMemo=true
DisableReadyPage=true
UsePreviousAppDir=false
DisableProgramGroupPage=true
UsePreviousGroup=false
RestartIfNeededByRun=false
ShowLanguageDialog=no
ChangesAssociations=yes

SetupIconFile=setup\IconFile.ico

[Languages]
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Dirs]
Name: {app}; Permissions: everyone-modify
Name: {app}\bin; Permissions: everyone-modify

[Files]
Source: "setup\EmulatorTM.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "setup\bin\*"; DestDir: "{app}\bin"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "setup\libusb-1.0 (dll)\*"; DestDir: "{app}\libusb-1.0 (dll)"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "setup\libusb-1.0.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "setup\libusb-1.0.exp"; DestDir: "{app}"; Flags: ignoreversion
Source: "setup\libusb-1.0.iobj"; DestDir: "{app}"; Flags: ignoreversion
Source: "setup\libusb-1.0.ipdb"; DestDir: "{app}"; Flags: ignoreversion
Source: "setup\libusb-1.0.lib"; DestDir: "{app}"; Flags: ignoreversion
Source: "setup\libusb-1.0.pdb"; DestDir: "{app}"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Registry]
Root: HKA; Subkey: "Software\Classes\.myp\OpenWithProgids"; ValueType: string; ValueName: "EmulatorTMFile.myp"; ValueData: ""; Flags: uninsdeletevalue
Root: HKA; Subkey: "Software\Classes\EmulatorTMFile.myp"; ValueType: string; ValueName: ""; ValueData: "EmulatorTM File"; Flags: uninsdeletekey
Root: HKA; Subkey: "Software\Classes\EmulatorTMFile.myp\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\EmulatorTM.exe,0"
Root: HKA; Subkey: "Software\Classes\EmulatorTMFile.myp\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\EmulatorTM.exe"" ""%1"""
Root: HKA; Subkey: "Software\Classes\Applications\EmulatorTM.exe\SupportedTypes"; ValueType: string; ValueName: ".myp"; ValueData: ""

[Icons]
Name: "{group}\EmulatorTM"; Filename: "{app}\EmulatorTM.exe"
Name: "{autodesktop}\EmulatorTM"; Filename: "{app}\EmulatorTM.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: runascurrentuser nowait postinstall skipifsilent

