;****************************************************************************
;**
;** Copyright (C) 2012-2014 Donna Whisnant, a.k.a. Dewtronics.
;** Contact: http://www.dewtronics.com/
;**
;** This file is part of the KJVCanOpener Application as originally written
;** and developed for Bethel Church, Festus, MO.
;**
;** GNU General Public License Usage
;** This file may be used under the terms of the GNU General Public License
;** version 3.0 as published by the Free Software Foundation and appearing
;** in the file gpl-3.0.txt included in the packaging of this file. Please
;** review the following information to ensure the GNU General Public License
;** version 3.0 requirements will be met:
;** http://www.gnu.org/copyleft/gpl.html.
;**
;** Other Usage
;** Alternatively, this file may be used in accordance with the terms and
;** conditions contained in a signed written agreement between you and
;** Dewtronics.
;**
;****************************************************************************

#define EXTRAVERSION "Preview"
#include "..\InnoSetup\version.iss"

[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"
Name: "fr"; MessagesFile: "compiler:Languages\French.isl"
Name: "es"; MessagesFile: "compiler:Languages\Spanish.isl"
Name: "de"; MessagesFile: "compiler:Languages\German.isl"

[Setup]
AppName=King James Pure Bible Search
AppId=KJVPureBibleSearch
AppMutex=KJVCanOpenerMutex
#expr VersionInstaller("..\build-KJVCanOpener-Qt_5_3_1_win32_dw2_4_8_0_rubenvb\release\KingJamesPureBibleSearch.exe")
AppCopyright=Copyright (C) 2012-2014 Donna Whisnant, a.k.a. Dewtronics.
AppPublisher=Dewtronics/Bethel Church
AppPublisherURL=http://www.dewtronics.com/
AppContact=Bethel Church
AppSupportURL=http://visitbethelchurch.com/
AppSupportPhone=(636)-931-3999
AppComments=King James Pure Bible Search ("Can Opener") Program.  Brought to you by the fervent prayers of Bethel Church, Festus, MO.
DefaultDirName={pf}\KJVPureBibleSearch
DefaultGroupName=King James Pure Bible Search
ShowLanguageDialog=auto
LicenseFile=LICENSE.txt
Compression=lzma
ChangesAssociations=yes
PrivilegesRequired=admin

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}";
Name: quicklaunchicon; Description: {cm:CreateQuickLaunchIcon}; GroupDescription: {cm:AdditionalIcons};

[InstallDelete]
; Remove Old Named Program:
Type: files; Name: "{app}\KJVCanOpener\app\KJVCanOpener.exe";
; Remove obsolete kjvuser database:
Type: files; Name: "{app}\KJVCanOpener\db\kjvuser.s3db";

[Files]
; app
Source: "..\build-KJVCanOpener-Qt_5_3_1_win32_dw2_4_8_0_rubenvb\release\KingJamesPureBibleSearch.exe"; DestDir: "{app}\KJVCanOpener\app"; Flags: ignoreversion;
Source: "..\build-KJVCanOpener-Qt_5_3_1_win32_dw2_4_8_0_rubenvb\release\Qt5Core.dll"; DestDir: "{app}\KJVCanOpener\app"; Flags: ignoreversion;
Source: "..\build-KJVCanOpener-Qt_5_3_1_win32_dw2_4_8_0_rubenvb\release\Qt5Gui.dll"; DestDir: "{app}\KJVCanOpener\app"; Flags: ignoreversion;
Source: "..\build-KJVCanOpener-Qt_5_3_1_win32_dw2_4_8_0_rubenvb\release\Qt5Widgets.dll"; DestDir: "{app}\KJVCanOpener\app"; Flags: ignoreversion;
Source: "..\build-KJVCanOpener-Qt_5_3_1_win32_dw2_4_8_0_rubenvb\release\Qt5Sql.dll"; DestDir: "{app}\KJVCanOpener\app"; Flags: ignoreversion;
Source: "..\build-KJVCanOpener-Qt_5_3_1_win32_dw2_4_8_0_rubenvb\release\Qt5Xml.dll"; DestDir: "{app}\KJVCanOpener\app"; Flags: ignoreversion;
Source: "..\build-KJVCanOpener-Qt_5_3_1_win32_dw2_4_8_0_rubenvb\release\Qt5Svg.dll"; DestDir: "{app}\KJVCanOpener\app"; Flags: ignoreversion;
Source: "..\build-KJVCanOpener-Qt_5_3_1_win32_dw2_4_8_0_rubenvb\release\Qt5Network.dll"; DestDir: "{app}\KJVCanOpener\app"; Flags: ignoreversion;
Source: "..\build-KJVCanOpener-Qt_5_3_1_win32_dw2_4_8_0_rubenvb\release\wwwidgets4.dll"; DestDir: "{app}\KJVCanOpener\app"; Flags: ignoreversion;
Source: "..\build-KJVCanOpener-Qt_5_3_1_win32_dw2_4_8_0_rubenvb\release\libstdc++-6.dll"; DestDir: "{app}\KJVCanOpener\app"; Flags: ignoreversion;
Source: "..\build-KJVCanOpener-Qt_5_3_1_win32_dw2_4_8_0_rubenvb\release\libgcc_s_dw2-1.dll"; DestDir: "{app}\KJVCanOpener\app"; Flags: ignoreversion;
Source: "..\build-KJVCanOpener-Qt_5_3_1_win32_dw2_4_8_0_rubenvb\release\libwinpthread-1.dll"; DestDir: "{app}\KJVCanOpener\app"; Flags: ignoreversion;

; doc
Source: "doc\KingJamesPureBibleSearch.pdf"; DestDir: "{app}\KJVCanOpener\doc"; Flags: ignoreversion;
Source: "kjvdatagen\kjv_summary.xls"; DestDir: "{app}\KJVCanOpener\doc"; Flags: ignoreversion;
Source: "articles\kjv_stats.xls"; DestDir: "{app}\KJVCanOpener\doc"; Flags: ignoreversion;

; examples:
Source: "examples\example01.kjs"; DestDir: "{app}\KJVCanOpener\examples"; Flags: ignoreversion;
Source: "examples\example02.kjs"; DestDir: "{app}\KJVCanOpener\examples"; Flags: ignoreversion;
Source: "examples\example03.kjs"; DestDir: "{app}\KJVCanOpener\examples"; Flags: ignoreversion;
Source: "examples\example04.kjs"; DestDir: "{app}\KJVCanOpener\examples"; Flags: ignoreversion;
Source: "examples\example05.kjs"; DestDir: "{app}\KJVCanOpener\examples"; Flags: ignoreversion;
Source: "examples\example06.kjs"; DestDir: "{app}\KJVCanOpener\examples"; Flags: ignoreversion;
Source: "examples\example07.kjs"; DestDir: "{app}\KJVCanOpener\examples"; Flags: ignoreversion;
Source: "examples\example08.kjs"; DestDir: "{app}\KJVCanOpener\examples"; Flags: ignoreversion;
Source: "examples\example09.kjs"; DestDir: "{app}\KJVCanOpener\examples"; Flags: ignoreversion;
Source: "examples\example10.kjs"; DestDir: "{app}\KJVCanOpener\examples"; Flags: ignoreversion;
Source: "examples\example11.kjs"; DestDir: "{app}\KJVCanOpener\examples"; Flags: ignoreversion;
Source: "examples\example12.kjs"; DestDir: "{app}\KJVCanOpener\examples"; Flags: ignoreversion;
Source: "examples\example13.kjs"; DestDir: "{app}\KJVCanOpener\examples"; Flags: ignoreversion;
Source: "examples\example14.kjs"; DestDir: "{app}\KJVCanOpener\examples"; Flags: ignoreversion;
Source: "examples\example15.kjs"; DestDir: "{app}\KJVCanOpener\examples"; Flags: ignoreversion;
Source: "examples\example16.kjs"; DestDir: "{app}\KJVCanOpener\examples"; Flags: ignoreversion;
Source: "examples\example17.kjs"; DestDir: "{app}\KJVCanOpener\examples"; Flags: ignoreversion;
Source: "examples\example18.kjs"; DestDir: "{app}\KJVCanOpener\examples"; Flags: ignoreversion;

; platforms:
Source: "..\build-KJVCanOpener-Qt_5_3_1_win32_dw2_4_8_0_rubenvb\release\platforms\qwindows.dll"; DestDir: "{app}\KJVCanOpener\app\platforms"; Flags: ignoreversion;
Source: "..\build-KJVCanOpener-Qt_5_3_1_win32_dw2_4_8_0_rubenvb\release\platforms\qminimal.dll"; DestDir: "{app}\KJVCanOpener\app\platforms"; Flags: ignoreversion;
Source: "..\build-KJVCanOpener-Qt_5_3_1_win32_dw2_4_8_0_rubenvb\release\platforms\qoffscreen.dll"; DestDir: "{app}\KJVCanOpener\app\platforms"; Flags: ignoreversion;

; plugins/sqldrivers
;Source: ".\plugins\sqldrivers\qsqlite.dll"; DestDir: "{app}\KJVCanOpener\plugins\sqldrivers"; Flags: ignoreversion;

; plugins/imageformats
Source: ".\plugins\imageformats\qdds.dll"; DestDir: "{app}\KJVCanOpener\plugins\imageformats"; Flags: ignoreversion;
Source: ".\plugins\imageformats\qgif.dll"; DestDir: "{app}\KJVCanOpener\plugins\imageformats"; Flags: ignoreversion;
Source: ".\plugins\imageformats\qicns.dll"; DestDir: "{app}\KJVCanOpener\plugins\imageformats"; Flags: ignoreversion;
Source: ".\plugins\imageformats\qico.dll"; DestDir: "{app}\KJVCanOpener\plugins\imageformats"; Flags: ignoreversion;
Source: ".\plugins\imageformats\qjp2.dll"; DestDir: "{app}\KJVCanOpener\plugins\imageformats"; Flags: ignoreversion;
Source: ".\plugins\imageformats\qjpeg.dll"; DestDir: "{app}\KJVCanOpener\plugins\imageformats"; Flags: ignoreversion;
Source: ".\plugins\imageformats\qmng.dll"; DestDir: "{app}\KJVCanOpener\plugins\imageformats"; Flags: ignoreversion;
Source: ".\plugins\imageformats\qsvg.dll"; DestDir: "{app}\KJVCanOpener\plugins\imageformats"; Flags: ignoreversion;
Source: ".\plugins\imageformats\qtga.dll"; DestDir: "{app}\KJVCanOpener\plugins\imageformats"; Flags: ignoreversion;
Source: ".\plugins\imageformats\qtiff.dll"; DestDir: "{app}\KJVCanOpener\plugins\imageformats"; Flags: ignoreversion;
Source: ".\plugins\imageformats\qwbmp.dll"; DestDir: "{app}\KJVCanOpener\plugins\imageformats"; Flags: ignoreversion;
Source: ".\plugins\imageformats\qwebp.dll"; DestDir: "{app}\KJVCanOpener\plugins\imageformats"; Flags: ignoreversion;

; plugins/accessible
Source: ".\plugins\accessible\qtaccessiblewidgets.dll"; DestDir: "{app}\KJVCanOpener\plugins\accessible"; Flags: ignoreversion;

; plugins/styles
Source: ".\plugins\styles\qcleanlooksstyle.dll"; DestDir: "{app}\KJVCanOpener\plugins\styles"; Flags: ignoreversion;
Source: ".\plugins\styles\qmotifstyle.dll"; DestDir: "{app}\KJVCanOpener\plugins\styles"; Flags: ignoreversion;
Source: ".\plugins\styles\qplastiquestyle.dll"; DestDir: "{app}\KJVCanOpener\plugins\styles"; Flags: ignoreversion;

; db
Source: "db\bbl-kjv1769.ccdb"; DestDir: "{app}\KJVCanOpener\db"; Flags: ignoreversion;
Source: "db\bbl-rvg2010.ccdb"; DestDir: "{app}\KJVCanOpener\db"; Flags: ignoreversion;
Source: "db\dct-web1828.s3db"; DestDir: "{app}\KJVCanOpener\db"; Flags: ignoreversion;

; license
Source: "gpl-3.0.txt"; DestDir: "{app}\KJVCanOpener\license"; Flags: ignoreversion;

; fonts
Source: "fonts\SCRIPTBL.TTF"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: "fonts\DejaVuSans-Bold.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: "fonts\DejaVuSans-BoldOblique.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: "fonts\DejaVuSans-ExtraLight.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: "fonts\DejaVuSans-Oblique.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: "fonts\DejaVuSans.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: "fonts\DejaVuSansCondensed-Bold.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: "fonts\DejaVuSansCondensed-BoldOblique.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: "fonts\DejaVuSansCondensed-Oblique.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: "fonts\DejaVuSansCondensed.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: "fonts\DejaVuSansMono-Bold.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: "fonts\DejaVuSansMono-BoldOblique.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: "fonts\DejaVuSansMono-Oblique.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: "fonts\DejaVuSansMono.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: "fonts\DejaVuSerif-Bold.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: "fonts\DejaVuSerif-BoldItalic.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: "fonts\DejaVuSerif-Italic.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: "fonts\DejaVuSerif.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: "fonts\DejaVuSerifCondensed-Bold.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: "fonts\DejaVuSerifCondensed-BoldItalic.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: "fonts\DejaVuSerifCondensed-Italic.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: "fonts\DejaVuSerifCondensed.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;

; translations
Source: "translations\kjpbs.en.qm"; DestDir: "{app}\KJVCanOpener\translations"; Flags: ignoreversion;
;Source: "translations\kjpbs.es.qm"; DestDir: "{app}\KJVCanOpener\translations"; Flags: ignoreversion;
;Source: "translations\kjpbs.fr.qm"; DestDir: "{app}\KJVCanOpener\translations"; Flags: ignoreversion;
;Source: "translations\kjpbs.de.qm"; DestDir: "{app}\KJVCanOpener\translations"; Flags: ignoreversion;

[Registry]
Root: HKCR; Subkey: ".kjs"; ValueType: string; ValueName: ""; ValueData: "KJVCanOpener"; Flags: uninsdeletevalue;
Root: HKCR; Subkey: "KJVCanOpener"; ValueType: string; ValueName: ""; ValueData: "KJVCanOpener"; Flags: uninsdeletekey;
Root: HKCR; Subkey: "KJVCanOpener\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\KJVCanOpener\app\KingJamesPureBibleSearch.exe,0";
Root: HKCR; Subkey: "KJVCanOpener\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\KJVCanOpener\app\KingJamesPureBibleSearch.exe"" ""%1""";

[Icons]
Name: "{group}\{#SetupSetting("AppName")}"; Filename: "{app}\KJVCanOpener\app\KingJamesPureBibleSearch.exe";
Name: "{group}\{cm:UninstallProgram,{#SetupSetting("AppName")}}"; Filename: "{uninstallexe}";
Name: "{group}\King James Pure Bible Search User Manual"; Filename: "{app}\KJVCanOpener\doc\KingJamesPureBibleSearch.pdf";
Name: "{group}\King James Statistics"; Filename: "{app}\KJVCanOpener\doc\kjv_stats.xls";
Name: "{group}\King James Word Summary"; Filename: "{app}\KJVCanOpener\doc\kjv_summary.xls";
Name: "{group}\Search Examples"; Filename: "{app}\KJVCanOpener\examples";
Name: "{commondesktop}\{#SetupSetting("AppName")}"; Filename: "{app}\KJVCanOpener\app\KingJamesPureBibleSearch.exe"; Tasks: desktopicon;
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\{#SetupSetting("AppName")}"; Filename: "{app}\KJVCanOpener\app\KingJamesPureBibleSearch.exe"; Tasks: quicklaunchicon;

[Run]
Filename: {app}\KJVCanOpener\app\KingJamesPureBibleSearch.exe; Description: {cm:LaunchProgram,{#SetupSetting("AppName")}}; Flags: nowait postinstall skipifsilent

