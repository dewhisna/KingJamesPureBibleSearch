/****************************************************************************
**
** Copyright (C) 2014-2022 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef DICT_DATABASE_INFO_DLG_H
#define DICT_DATABASE_INFO_DLG_H

#include "dbstruct.h"

#include <QWidget>
#include <QDialog>
#include <QPointer>

// ============================================================================

#include "ui_DictDatabaseInfoDlg.h"

class CDictDatabaseInfoDialog : public QDialog
{
	Q_OBJECT

public:
	CDictDatabaseInfoDialog(CDictionaryDatabasePtr pDictDatabase, QWidget *parent = nullptr);
	~CDictDatabaseInfoDialog();

// UI Private:
private:
	Ui::CDictDatabaseInfoDialog ui;
};

// ============================================================================

// SmartPointer classes needed, particularly for stack instantiated dialogs, since
//		this dialog is only WindowModal and the parent can get deleted during an
//		app close event, causing an attempted double-free which leads to a crash:
class CDictDatabaseInfoDialogPtr : public QPointer<CDictDatabaseInfoDialog>
{
public:
	CDictDatabaseInfoDialogPtr(CDictionaryDatabasePtr pDictDatabase, QWidget *parent = nullptr)
		:	QPointer<CDictDatabaseInfoDialog>(new CDictDatabaseInfoDialog(pDictDatabase, parent))
	{

	}

	virtual ~CDictDatabaseInfoDialogPtr()
	{
		if (!isNull()) delete data();
	}
};

// ============================================================================

#endif	// DICT_DATABASE_INFO_DLG_H
