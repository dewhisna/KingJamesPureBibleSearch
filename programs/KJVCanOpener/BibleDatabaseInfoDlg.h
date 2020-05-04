/****************************************************************************
**
** Copyright (C) 2014-2020 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef BIBLE_DATABASE_INFO_DLG_H
#define BIBLE_DATABASE_INFO_DLG_H

#include "dbstruct.h"

#include <QWidget>
#include <QDialog>
#include <QPointer>

// ============================================================================

#include "ui_BibleDatabaseInfoDlg.h"

class CBibleDatabaseInfoDialog : public QDialog
{
	Q_OBJECT

public:
	CBibleDatabaseInfoDialog(CBibleDatabasePtr pBibleDatabase, QWidget *parent = NULL);
	~CBibleDatabaseInfoDialog();

// UI Private:
private:
	Ui::CBibleDatabaseInfoDialog ui;
};

// ============================================================================

// SmartPointer classes needed, particularly for stack instantiated dialogs, since
//		this dialog is only WindowModal and the parent can get deleted during an
//		app close event, causing an attempted double-free which leads to a crash:
class CBibleDatabaseInfoDialogPtr : public QPointer<CBibleDatabaseInfoDialog>
{
public:
	CBibleDatabaseInfoDialogPtr(CBibleDatabasePtr pBibleDatabase, QWidget *parent = NULL)
		:	QPointer<CBibleDatabaseInfoDialog>(new CBibleDatabaseInfoDialog(pBibleDatabase, parent))
	{

	}

	virtual ~CBibleDatabaseInfoDialogPtr()
	{
		if (!isNull()) delete data();
	}
};

// ============================================================================

#endif	// BIBLE_DATABASE_INFO_DLG_H
