/****************************************************************************
**
** Copyright (C) 2014-2025 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef EDIT_WWW_LINK_DLG_H
#define EDIT_WWW_LINK_DLG_H

#include <QDialog>
#include <QString>
#include <QPointer>

// ============================================================================

#include "ui_EditWWWLinkDlg.h"


class CEditWWWLinkDlg : public QDialog
{
	Q_OBJECT

public:
	explicit CEditWWWLinkDlg(const QString strURL, QWidget *pParent = nullptr);
	virtual ~CEditWWWLinkDlg();

	QString url() const { return m_strURL; }
	bool isValid() const { return m_bIsValid; }

public slots:
	virtual void accept() override;
	virtual void reject() override;

private slots:
	void en_textChanged(const QString &strURL);
	void en_unselect();

private:
	QString m_strURL;
	bool m_bIsValid;
	// ----
	Ui::CEditWWWLinkDlg ui;
};

// ============================================================================

// SmartPointer classes needed, particularly for stack instantiated dialogs, since
//		this dialog is only WindowModal and the parent can get deleted during an
//		app close event, causing an attempted double-free which leads to a crash:
class CEditWWWLinkDlgPtr : public QPointer<CEditWWWLinkDlg>
{
public:
	CEditWWWLinkDlgPtr(const QString strURL, QWidget *pParent = nullptr)
		:	QPointer<CEditWWWLinkDlg>(new CEditWWWLinkDlg(strURL, pParent))
	{

	}

	virtual ~CEditWWWLinkDlgPtr()
	{
		if (!isNull()) delete data();
	}
};

// ============================================================================

#endif	// EDIT_WWW_LINK_DLG_H
