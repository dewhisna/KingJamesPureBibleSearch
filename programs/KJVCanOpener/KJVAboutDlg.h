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

namespace Ui {
class CKJVAboutDlg;
}

class CKJVAboutDlg : public QDialog
{
	Q_OBJECT

public:
	explicit CKJVAboutDlg(QWidget *parent = 0);
	~CKJVAboutDlg();

private slots:
	void en_licenseDisplay();

// Data Private:
private:


// UI Private:
private:
	Ui::CKJVAboutDlg *ui;
	QGraphicsPixmapItem *m_pBethelChurch;
	QGraphicsTextItem *m_pAppTitle;
	QGraphicsTextItem *m_pAppSpecialVersion;
	QGraphicsTextItem *m_pBroughtToYouBy;
	QGraphicsTextItem *m_pBethelURL;
};

#endif // KJVABOUTDLG_H
