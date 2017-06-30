/****************************************************************************
**
** Copyright (C) 2012 Donna Whisnant, a.k.a. Dewtronics.
** Contact: http://www.dewtronics.com/
**
** This file is part of the KJVCanOpener Application as originally written
** and developed for Bethel Church, Festus, MO.
**
** GNU General Public License Usage
** This file may be used under the terms of the GNU General Public License
** version 3.0 as published by the Free Software Foundation and appearing
** in the file gpl-3.0.txt included in the packaging of this file. Please
** review the following information to ensure the GNU General Public License
** version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and
** Dewtronics.
**
****************************************************************************/

#ifndef VERSION_H
#define VERSION_H

#define VER_QT						"3.0.90"
#define VER_FILEVERSION				3,0,90,0
#define VER_FILEVERSION_STR			"3,0,90,0\0"

#define VER_PRODUCTVERSION			3,0,90,0
#define VER_PRODUCTVERSION_STR		"3,0,90,0\0"

#if defined(EMSCRIPTEN)
#define VER_SPECIALVERSION_STR		"(Emscripten Web-Version)\0"
#define SPECIAL_BUILD				1
#elif defined(VNCSERVER)
#define VER_SPECIALVERSION_STR		"(VNC Web-Version)\0"
#define SPECIAL_BUILD				1
#else
#define VER_SPECIALVERSION_STR		"Alpha Test Version\0"
#define SPECIAL_BUILD				1
#endif
#define VER_BUILD_DATE_STR			__DATE__
#define VER_BUILD_TIME_STR			__TIME__

#define VER_APPNAME_STR_QT			"KingJamesPureBibleSearch\0"
#define VER_ORGNAME_STR_QT			VER_COMPANYNAME_STR
#define VER_ORGDOMAIN_STR_QT		VER_COMPANYDOMAIN_STR

#define VER_COMMENTS_STR			"Don't make me have to open a can on you!\n\0"
#define VER_COMPANYNAME_STR			"Dewtronics\0"
#define VER_FILEDESCRIPTION_STR		"King James Pure Bible Search\0"
#define VER_INTERNALNAME_STR		"King James Pure Bible Search\0"
#define VER_LEGALCOPYRIGHT_STR		"Copyright(c)2012-2017 Donna Whisnant, a.k.a. Dewtronics\0"
#define VER_ORIGINALFILENAME_STR	"KingJamesPureBibleSearch.exe\0"
#define VER_PRODUCTNAME_STR			"King James Pure Bible Search\0"

#define VER_COMPANYDOMAIN_STR		"dewtronics.com\0"

#define VER_URL_STR					"http://visitbethelchurch.com/\0"

#endif // VERSION_H
