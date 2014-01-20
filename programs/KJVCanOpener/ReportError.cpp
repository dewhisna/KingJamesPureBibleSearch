/****************************************************************************
**
** Copyright (C) 2014 Donna Whisnant, a.k.a. Dewtronics.
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

#include "ReportError.h"

#include <QDebug>

// ============================================================================


QMessageBox::StandardButton displayWarning(QWidget *pParent, const QString &strTitle, const QString &strText,
										   QMessageBox::StandardButtons nButtons,
										   QMessageBox::StandardButton nDefaultButton)
{
#ifndef EMSCRIPTEN
	return QMessageBox::warning(pParent, strTitle, strText, nButtons, nDefaultButton);
#else
	qDebug("warning: %s: %s", strTitle.toUtf8().data(), strText.toUtf8().data());
	Q_UNUSED(pParent);
	Q_UNUSED(nButtons);
	if (nDefaultButton != QMessageBox::NoButton) return nDefaultButton;
	return QMessageBox::Ok;
#endif
}

QMessageBox::StandardButton displayWarning(QWidget *pParent, const QString &strTitle, const QString &strText,
											QMessageBox::StandardButton nButton0,
											QMessageBox::StandardButton nButton1)
{
	return displayWarning(pParent, strTitle, strText, QMessageBox::StandardButtons(nButton0 | nButton1), nButton1);
}

// ----------------------------------------------------------------------------

QMessageBox::StandardButton displayInformation(QWidget *pParent, const QString &strTitle, const QString &strText,
												QMessageBox::StandardButtons nButtons,
												QMessageBox::StandardButton nDefaultButton)
{
#ifndef EMSCRIPTEN
	return QMessageBox::information(pParent, strTitle, strText, nButtons, nDefaultButton);
#else
	qDebug("information: %s: %s", strTitle.toUtf8().data(), strText.toUtf8().data());
	Q_UNUSED(pParent);
	Q_UNUSED(nButtons);
	if (nDefaultButton != QMessageBox::NoButton) return nDefaultButton;
	return QMessageBox::Ok;
#endif
}

QMessageBox::StandardButton displayInformation(QWidget *pParent, const QString &strTitle, const QString &strText,
												QMessageBox::StandardButton nButton0,
												QMessageBox::StandardButton nButton1)
{
	return displayInformation(pParent, strTitle, strText, QMessageBox::StandardButtons(nButton0 | nButton1), nButton1);
}

// ============================================================================
