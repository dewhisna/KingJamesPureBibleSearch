/****************************************************************************
**
** Copyright (C) 2014-2025 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef REPORT_ERROR_H
#define REPORT_ERROR_H

#ifndef IS_CONSOLE_APP
#include <QMessageBox>
#endif

#include <QString>
#include <functional>

// ============================================================================

#ifndef IS_CONSOLE_APP

#ifndef USE_ASYNC_DIALOGS
extern QMessageBox::StandardButton
#else
extern void
#endif
				displayWarning(QWidget *pParent, const QString &strTitle, const QString &strText,
								QMessageBox::StandardButtons nButtons = QMessageBox::Ok,
								QMessageBox::StandardButton nDefaultButton = QMessageBox::NoButton,
								std::function<void (QMessageBox::StandardButton nResult)> fnCompletion = nullptr);


#else

extern void displayWarning(void *pParent, const QString &strTitle, const QString &strText);

#endif

// ------------------------------------

#ifndef IS_CONSOLE_APP

#ifndef USE_ASYNC_DIALOGS
extern QMessageBox::StandardButton
#else
extern void
#endif
				displayInformation(QWidget *pParent, const QString &strTitle, const QString &strText,
									QMessageBox::StandardButtons nButtons = QMessageBox::Ok,
									QMessageBox::StandardButton nDefaultButton = QMessageBox::NoButton,
									std::function<void (QMessageBox::StandardButton nResult)> fnCompletion = nullptr);

#else

extern void displayInformation(void *pParent, const QString &strTitle, const QString &strText);

#endif

// ============================================================================

#endif		// REPORT_ERROR_H
