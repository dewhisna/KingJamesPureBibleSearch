/****************************************************************************
**
** Copyright (C) 2022 Donna Whisnant, a.k.a. Dewtronics.
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

#include "GeoMap.h"

#include <QQmlApplicationEngine>
#include <QUrl>
#include <QStringLiteral>
#include <QList>
#include <QEvent>

// ============================================================================

CGeoMap::CGeoMap(QObject *pParent)
	:	QObject(pParent),
		m_pQmlEngine(new QQmlApplicationEngine(this))
{
	connect(m_pQmlEngine, SIGNAL(objectCreated(QObject*,QUrl)), this, SLOT(en_objectCreated(QObject*,QUrl)));
}

CGeoMap::~CGeoMap()
{
	closeGeoMaps();
}

bool CGeoMap::eventFilter(QObject *watched, QEvent *event)
{
	Q_ASSERT(watched != nullptr);
	Q_ASSERT(event != nullptr);

	if (event->type() == QEvent::Close) {
		watched->deleteLater();
		emit closedGeoMap();
		return false;		// Continue to pass event
	}

	return QObject::eventFilter(watched, event);
}

void CGeoMap::setWindowTitle(const QString &strTitle)
{
	if (!m_pGeoMap.isNull()) {
		bool bValidProp = m_pGeoMap->setProperty("title", QVariant::fromValue<QString>(strTitle));
		Q_ASSERT(bValidProp);
	}
	m_strTitle = strTitle;
}

void CGeoMap::displayGeoMap(bool bShow)
{
	if (bShow) {
		if (m_pGeoMap.isNull()) {
			if (!m_pQmlEngine.isNull()) {
				if (m_pQmlEngine->rootObjects().isEmpty()) {
					m_pQmlEngine->load(QUrl(QStringLiteral("qrc:/res/geomap/geomap.qml")));
				}
			}
		} else {
			bool bValidProp = m_pGeoMap->setProperty("visible", QVariant::fromValue<bool>(true));
			Q_ASSERT(bValidProp);
		}
	} else {
		if (!m_pGeoMap.isNull()) {
			bool bValidProp = m_pGeoMap->setProperty("visible", QVariant::fromValue<bool>(false));
			Q_ASSERT(bValidProp);
		}
	}
}

void CGeoMap::en_objectCreated(QObject *pObject, const QUrl &url)
{
	Q_UNUSED(url);

	m_pGeoMap = pObject;
	if (pObject) {
		pObject->setParent(parent());
		pObject->installEventFilter(this);

		bool bValidProp = pObject->setProperty("title", QVariant::fromValue<QString>(m_strTitle));
		Q_ASSERT(bValidProp);
	}
}

void CGeoMap::closeGeoMaps()
{
	if (m_pQmlEngine.isNull()) return;

	QList<QObject*> lstObjects = m_pQmlEngine->rootObjects();
	for (auto itr : lstObjects) {
		delete itr;
	}
	delete m_pQmlEngine;
}

// ============================================================================
