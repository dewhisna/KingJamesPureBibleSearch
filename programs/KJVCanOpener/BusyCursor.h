/****************************************************************************
**
** Copyright (C) 2013 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef BUSY_CURSOR_H
#define BUSY_CURSOR_H

#include <QApplication>
#include <QWidget>
#include <QCursor>

class CBusyCursor
{
public:
	CBusyCursor(QWidget *pWidget)
		:	m_pWidget(pWidget)
	{
		if (m_pWidget) {
			m_originalCursor = m_pWidget->cursor();
			m_pWidget->setCursor(Qt::BusyCursor);
		} else {
			QApplication::setOverrideCursor(Qt::BusyCursor);
		}
	}

	~CBusyCursor()
	{
		if (m_pWidget) {
			m_pWidget->setCursor(m_originalCursor);
		} else {
			QApplication::restoreOverrideCursor();
		}
	}

private:
	QCursor m_originalCursor;
	QWidget *m_pWidget;
};

#endif	// BUSY_CURSOR_H

