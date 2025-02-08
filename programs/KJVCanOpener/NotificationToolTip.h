/****************************************************************************
**
** Copyright (C) 2013-2025 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef NOTIFICATION_TOOL_TIP_H
#define NOTIFICATION_TOOL_TIP_H

#include <QObject>

// Forward Declarations:
class QPoint;
class QString;
class QWidget;

// ============================================================================

class CNotificationToolTip : public QObject
{
	Q_OBJECT

public:
	CNotificationToolTip(int nDisplayTimeMS, const QPoint &ptPos, const QString &strMessage, QWidget *pWidget = nullptr);
	virtual ~CNotificationToolTip();

private slots:
	void en_hideMessage();

};

// ============================================================================

#endif	// NOTIFICATION_TOOL_TIP_H

