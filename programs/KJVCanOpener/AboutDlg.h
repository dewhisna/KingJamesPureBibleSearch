/****************************************************************************
**
** Copyright (C) 2012-2025 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef ABOUT_DIALOG_H
#define ABOUT_DIALOG_H

#include <QDialog>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>
#include <QPointer>

// ============================================================================

#include "ui_AboutDlg.h"

class CAboutDlg : public QDialog
{
	Q_OBJECT

public:
	explicit CAboutDlg(QWidget *parent);
	virtual ~CAboutDlg();

private slots:
	void en_resizeMe();
	void en_licenseDisplay();

// Data Private:
private:


// UI Private:
private:
	QGraphicsPixmapItem *m_pBethelChurch;
	QGraphicsTextItem *m_pAppTitle;
	QGraphicsTextItem *m_pExtraVersionInfo;
	QGraphicsTextItem *m_pAppSpecialVersion;
	QGraphicsTextItem *m_pAppBuildDateTime;
	QGraphicsTextItem *m_pQtVersion;
	QGraphicsTextItem *m_pBroughtToYouBy;
	QGraphicsTextItem *m_pBethelURL;

	Ui::CAboutDlg ui;
};

// ============================================================================

#endif // ABOUT_DIALOG_H
