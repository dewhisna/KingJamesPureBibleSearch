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

#ifndef PERSISTENTSETTINGS_H
#define PERSISTENTSETTINGS_H

#include <QObject>
#include <QSettings>


extern QString groupCombine(const QString &strSubgroup, const QString &strGroup);


class CPersistentSettings : public QObject
{
	Q_OBJECT
private:				// Enforce Singleton:
	CPersistentSettings(QObject *parent = 0);

public:
	~CPersistentSettings();
	static CPersistentSettings *instance();
	inline QSettings &settings() { return *m_pSettings; }

signals:

public slots:


private:
	QSettings *m_pSettings;
};

#endif // PERSISTENTSETTINGS_H
