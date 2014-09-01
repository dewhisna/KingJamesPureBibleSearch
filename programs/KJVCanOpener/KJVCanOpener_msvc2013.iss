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

#define EXTRAVERSION "Preview2"
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
#expr VersionInstaller(".\winbuild\KJVCanOpener\app\KingJamesPureBibleSearch.exe")
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
Source: ".\winbuild\KJVCanOpener\app\KingJamesPureBibleSearch.exe"; DestDir: "{app}\KJVCanOpener\app"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\app\Qt5Core.dll"; DestDir: "{app}\KJVCanOpener\app"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\app\Qt5Gui.dll"; DestDir: "{app}\KJVCanOpener\app"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\app\Qt5Widgets.dll"; DestDir: "{app}\KJVCanOpener\app"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\app\Qt5Sql.dll"; DestDir: "{app}\KJVCanOpener\app"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\app\Qt5Xml.dll"; DestDir: "{app}\KJVCanOpener\app"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\app\Qt5Svg.dll"; DestDir: "{app}\KJVCanOpener\app"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\app\Qt5Network.dll"; DestDir: "{app}\KJVCanOpener\app"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\app\wwwidgets4.dll"; DestDir: "{app}\KJVCanOpener\app"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\app\msvcp120.dll"; DestDir: "{app}\KJVCanOpener\app"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\app\msvcr120.dll"; DestDir: "{app}\KJVCanOpener\app"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\app\vccorlib120.dll"; DestDir: "{app}\KJVCanOpener\app"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\app\icudt52.dll"; DestDir: "{app}\KJVCanOpener\app"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\app\icuin52.dll"; DestDir: "{app}\KJVCanOpener\app"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\app\icuuc52.dll"; DestDir: "{app}\KJVCanOpener\app"; Flags: ignoreversion;

; doc
Source: ".\winbuild\KJVCanOpener\doc\KingJamesPureBibleSearch.pdf"; DestDir: "{app}\KJVCanOpener\doc"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\doc\kjv_summary.xls"; DestDir: "{app}\KJVCanOpener\doc"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\doc\kjv_stats.xls"; DestDir: "{app}\KJVCanOpener\doc"; Flags: ignoreversion;

; examples:
Source: ".\winbuild\KJVCanOpener\examples\example01.kjs"; DestDir: "{app}\KJVCanOpener\examples"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\examples\example02.kjs"; DestDir: "{app}\KJVCanOpener\examples"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\examples\example03.kjs"; DestDir: "{app}\KJVCanOpener\examples"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\examples\example04.kjs"; DestDir: "{app}\KJVCanOpener\examples"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\examples\example05.kjs"; DestDir: "{app}\KJVCanOpener\examples"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\examples\example06.kjs"; DestDir: "{app}\KJVCanOpener\examples"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\examples\example07.kjs"; DestDir: "{app}\KJVCanOpener\examples"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\examples\example08.kjs"; DestDir: "{app}\KJVCanOpener\examples"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\examples\example09.kjs"; DestDir: "{app}\KJVCanOpener\examples"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\examples\example10.kjs"; DestDir: "{app}\KJVCanOpener\examples"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\examples\example11.kjs"; DestDir: "{app}\KJVCanOpener\examples"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\examples\example12.kjs"; DestDir: "{app}\KJVCanOpener\examples"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\examples\example13.kjs"; DestDir: "{app}\KJVCanOpener\examples"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\examples\example14.kjs"; DestDir: "{app}\KJVCanOpener\examples"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\examples\example15.kjs"; DestDir: "{app}\KJVCanOpener\examples"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\examples\example16.kjs"; DestDir: "{app}\KJVCanOpener\examples"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\examples\example17.kjs"; DestDir: "{app}\KJVCanOpener\examples"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\examples\example18.kjs"; DestDir: "{app}\KJVCanOpener\examples"; Flags: ignoreversion;

; platforms:
Source: ".\winbuild\KJVCanOpener\app\platforms\qwindows.dll"; DestDir: "{app}\KJVCanOpener\app\platforms"; Flags: ignoreversion;
;Source: ".\winbuild\KJVCanOpener\app\platforms\qminimal.dll"; DestDir: "{app}\KJVCanOpener\app\platforms"; Flags: ignoreversion;
;Source: ".\winbuild\KJVCanOpener\app\platforms\qoffscreen.dll"; DestDir: "{app}\KJVCanOpener\app\platforms"; Flags: ignoreversion;

; plugins/accessible
Source: ".\winbuild\KJVCanOpener\plugins\accessible\qtaccessiblewidgets.dll"; DestDir: "{app}\KJVCanOpener\plugins\accessible"; Flags: ignoreversion;

; plugins/bearer
Source: ".\winbuild\KJVCanOpener\plugins\bearer\qgenericbearer.dll"; DestDir: "{app}\KJVCanOpener\plugins\bearer"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\plugins\bearer\qnativewifibearer.dll"; DestDir: "{app}\KJVCanOpener\plugins\bearer"; Flags: ignoreversion;

; plugins/iconengines
Source: ".\winbuild\KJVCanOpener\plugins\iconengines\qsvgicon.dll"; DestDir: "{app}\KJVCanOpener\plugins\iconengines"; Flags: ignoreversion;

; plugins/imageformats
Source: ".\winbuild\KJVCanOpener\plugins\imageformats\qdds.dll"; DestDir: "{app}\KJVCanOpener\plugins\imageformats"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\plugins\imageformats\qgif.dll"; DestDir: "{app}\KJVCanOpener\plugins\imageformats"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\plugins\imageformats\qicns.dll"; DestDir: "{app}\KJVCanOpener\plugins\imageformats"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\plugins\imageformats\qico.dll"; DestDir: "{app}\KJVCanOpener\plugins\imageformats"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\plugins\imageformats\qjp2.dll"; DestDir: "{app}\KJVCanOpener\plugins\imageformats"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\plugins\imageformats\qjpeg.dll"; DestDir: "{app}\KJVCanOpener\plugins\imageformats"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\plugins\imageformats\qmng.dll"; DestDir: "{app}\KJVCanOpener\plugins\imageformats"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\plugins\imageformats\qsvg.dll"; DestDir: "{app}\KJVCanOpener\plugins\imageformats"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\plugins\imageformats\qtga.dll"; DestDir: "{app}\KJVCanOpener\plugins\imageformats"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\plugins\imageformats\qtiff.dll"; DestDir: "{app}\KJVCanOpener\plugins\imageformats"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\plugins\imageformats\qwbmp.dll"; DestDir: "{app}\KJVCanOpener\plugins\imageformats"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\plugins\imageformats\qwebp.dll"; DestDir: "{app}\KJVCanOpener\plugins\imageformats"; Flags: ignoreversion;

; plugins/sqldrivers
Source: ".\winbuild\KJVCanOpener\plugins\sqldrivers\qsqlite.dll"; DestDir: "{app}\KJVCanOpener\plugins\sqldrivers"; Flags: ignoreversion;

; plugins/styles
;Source: ".\winbuild\KJVCanOpener\plugins\styles\qcleanlooksstyle.dll"; DestDir: "{app}\KJVCanOpener\plugins\styles"; Flags: ignoreversion;
;Source: ".\winbuild\KJVCanOpener\plugins\styles\qmotifstyle.dll"; DestDir: "{app}\KJVCanOpener\plugins\styles"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\plugins\styles\qplastiquestyle.dll"; DestDir: "{app}\KJVCanOpener\plugins\styles"; Flags: ignoreversion;

; db
Source: ".\winbuild\KJVCanOpener\db\bbl-kjv1769.ccdb"; DestDir: "{app}\KJVCanOpener\db"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\db\bbl-rvg2010.ccdb"; DestDir: "{app}\KJVCanOpener\db"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\db\dct-web1828.s3db"; DestDir: "{app}\KJVCanOpener\db"; Flags: ignoreversion;

; license
Source: ".\winbuild\KJVCanOpener\license\gpl-3.0.txt"; DestDir: "{app}\KJVCanOpener\license"; Flags: ignoreversion;

; fonts
Source: ".\winbuild\KJVCanOpener\fonts\SCRIPTBL.TTF"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\fonts\DejaVuSans-Bold.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\fonts\DejaVuSans-BoldOblique.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\fonts\DejaVuSans-ExtraLight.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\fonts\DejaVuSans-Oblique.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\fonts\DejaVuSans.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\fonts\DejaVuSansCondensed-Bold.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\fonts\DejaVuSansCondensed-BoldOblique.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\fonts\DejaVuSansCondensed-Oblique.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\fonts\DejaVuSansCondensed.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\fonts\DejaVuSansMono-Bold.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\fonts\DejaVuSansMono-BoldOblique.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\fonts\DejaVuSansMono-Oblique.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\fonts\DejaVuSansMono.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\fonts\DejaVuSerif-Bold.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\fonts\DejaVuSerif-BoldItalic.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\fonts\DejaVuSerif-Italic.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\fonts\DejaVuSerif.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\fonts\DejaVuSerifCondensed-Bold.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\fonts\DejaVuSerifCondensed-BoldItalic.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\fonts\DejaVuSerifCondensed-Italic.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\fonts\DejaVuSerifCondensed.ttf"; DestDir: "{app}\KJVCanOpener\fonts"; Flags: ignoreversion;

; translations
Source: ".\winbuild\KJVCanOpener\translations\kjpbs.en.qm"; DestDir: "{app}\KJVCanOpener\translations"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\translations\kjpbs.es.qm"; DestDir: "{app}\KJVCanOpener\translations"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\translations\kjpbs.fr.qm"; DestDir: "{app}\KJVCanOpener\translations"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\translations\kjpbs.de.qm"; DestDir: "{app}\KJVCanOpener\translations"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\translations\qt_es.qm"; DestDir: "{app}\KJVCanOpener\translations"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\translations\qt_fr.qm"; DestDir: "{app}\KJVCanOpener\translations"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\translations\qt_de.qm"; DestDir: "{app}\KJVCanOpener\translations"; Flags: ignoreversion;
Source: ".\winbuild\KJVCanOpener\translations\qtbase_de.qm"; DestDir: "{app}\KJVCanOpener\translations"; Flags: ignoreversion;

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

