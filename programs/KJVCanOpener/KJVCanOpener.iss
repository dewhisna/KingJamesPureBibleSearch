#include "C:\InnoSetup\version.iss"

[Setup]
AppName=KJVCanOpener
#expr VersionInstaller("..\KJVCanOpener-build-desktop-Qt_4_8_3\release\KJVCanOpener.exe")
AppPublisher=Bethel Church
DefaultDirName={pf}\KJVCanOpener
DefaultGroupName=KJVCanOpener
ShowLanguageDialog=auto
LicenseFile=gpl-3.0.txt
Compression=lzma


[Files]
; app
Source: "..\KJVCanOpener-build-desktop-Qt_4_8_3\release\KJVCanOpener.exe"; DestDir: "{app}\app"; Flags: ignoreversion;
Source: {#GetEnv('QTDIR')}\bin\QtCore4.dll; DestDir: {app}\app; Flags: ignoreversion;
Source: {#GetEnv('QTDIR')}\bin\QtGui4.dll; DestDir: {app}\app; Flags: ignoreversion;
Source: {#GetEnv('QTDIR')}\bin\QtSql4.dll; DestDir: {app}\app; Flags: ignoreversion;
Source: {#GetEnv('QTDIR')}\..\..\..\MinGW\bin\libstdc++-6.dll; DestDir: {app}\app; Flags: ignoreversion;
Source: {#GetEnv('QTDIR')}\..\..\..\MinGW\bin\libgcc_s_dw2-1.dll; DestDir: {app}\app; Flags: ignoreversion;

; plugins/sqldrivers
Source: {#GetEnv('QTDIR')}\plugins\sqldrivers\qsqlite4.dll; DestDir: {app}\plugins\sqldrivers; Flags: ignoreversion;

; plugins/imageformats
Source: {#GetEnv('QTDIR')}\plugins\imageformats\qgif4.dll; DestDir: {app}\plugins\imageformats; Flags: ignoreversion;
Source: {#GetEnv('QTDIR')}\plugins\imageformats\qico4.dll; DestDir: {app}\plugins\imageformats; Flags: ignoreversion;
Source: {#GetEnv('QTDIR')}\plugins\imageformats\qjpeg4.dll; DestDir: {app}\plugins\imageformats; Flags: ignoreversion;
Source: {#GetEnv('QTDIR')}\plugins\imageformats\qmng4.dll; DestDir: {app}\plugins\imageformats; Flags: ignoreversion;
;Source: {#GetEnv('QTDIR')}\plugins\imageformats\qsvg4.dll; DestDir: {app}\plugins\imageformats; Flags: ignoreversion;
;Source: {#GetEnv('QTDIR')}\plugins\imageformats\qtga4.dll; DestDir: {app}\plugins\imageformats; Flags: ignoreversion;
;Source: {#GetEnv('QTDIR')}\plugins\imageformats\qtiff4.dll; DestDir: {app}\plugins\imageformats; Flags: ignoreversion;

; db
Source: "db\kjvtext.s3db"; DestDir: {app}\db; Flags: ignoreversion;
Source: "db\kjvuser.s3db"; DestDir: {app}\db; Flags: onlyifdoesntexist uninsneveruninstall; Permissions: users-modify;

; license
Source: "gpl-3.0.txt"; DestDir: {app}\license; Flags: ignoreversion;

; {fonts}
Source: "fonts/SCRIPTBL.TTF"; DestDir: "{fonts}"; FontInstall: "Script MT Bold"; Flags: onlyifdoesntexist uninsneveruninstall;

[Icons]
Name: {group}\{#SetupSetting("AppName")}; Filename: "{app}\app\KJVCanOpener.exe"

[Run]
Filename: {app}\app\KJVCanOpener.exe; Description: {cm:LaunchProgram,{#SetupSetting("AppName")}}; Flags: nowait postinstall skipifsilent

