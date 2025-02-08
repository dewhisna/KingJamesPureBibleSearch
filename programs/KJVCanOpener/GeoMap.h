/****************************************************************************
**
** Copyright (C) 2022-2025 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef GEO_MAP_H
#define GEO_MAP_H

#include <QObject>
#include <QPointer>

// Forward Declarations
class QQmlApplicationEngine;
class QUrl;
class QEvent;

// ============================================================================

class CGeoMap : public QObject
{
	Q_OBJECT

public:
	CGeoMap(QObject *pParent = nullptr);
	virtual ~CGeoMap();

	virtual bool eventFilter(QObject *watched, QEvent *event) override;

	QString windowTitle() const { return m_strTitle; }
	void setWindowTitle(const QString &strTitle);

public slots:
	void displayGeoMap(bool bShow = true);
	void closeGeoMaps();

signals:
	void closedGeoMap();

protected slots:
	void en_objectCreated(QObject *pObject, const QUrl &url);

private:
	QPointer<QQmlApplicationEngine> m_pQmlEngine;
	QPointer<QObject> m_pGeoMap;						// Actual QML GeoMap displayed by this CGeoMap object
	QString m_strTitle;									// Window Title
};

// ============================================================================

#endif	// GEO_MAP_H
