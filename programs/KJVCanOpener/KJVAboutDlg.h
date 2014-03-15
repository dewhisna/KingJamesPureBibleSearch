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

#ifndef KJVABOUTDLG_H
#define KJVABOUTDLG_H

#include <QDialog>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>
#include <QPointer>

// ============================================================================

#include "ui_KJVAboutDlg.h"

class CKJVAboutDlg : public QDialog
{
	Q_OBJECT

public:
	explicit CKJVAboutDlg(QWidget *parent);
	virtual ~CKJVAboutDlg();

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
	QGraphicsTextItem *m_pQtVersion;
	QGraphicsTextItem *m_pBroughtToYouBy;
	QGraphicsTextItem *m_pBethelURL;

	Ui::CKJVAboutDlg ui;
};

// ============================================================================

// SmartPointer classes needed, particularly for stack instantiated dialogs, since
//		this dialog is only WindowModal and the parent can get deleted during an
//		app close event, causing an attempted double-free which leads to a crash:
class CKJVAboutDlgPtr : public QPointer<CKJVAboutDlg>
{
public:
	CKJVAboutDlgPtr(QWidget *parent = NULL)
		:	QPointer<CKJVAboutDlg>(new CKJVAboutDlg(parent))
	{

	}

	virtual ~CKJVAboutDlgPtr()
	{
		if (!isNull()) delete data();
	}
};

// ============================================================================

#endif // KJVABOUTDLG_H
