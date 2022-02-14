/****************************************************************************
**
** Copyright (C) 2012-2020 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef PASSAGE_NAVIGATOR_DLG_H
#define PASSAGE_NAVIGATOR_DLG_H

#include "dbstruct.h"
#include "PassageNavigator.h"

#include <QDialog>
#include <QAbstractButton>
#include <QPushButton>
#include <QPointer>

// ============================================================================

#include "ui_PassageNavigatorDlg.h"

class CPassageNavigatorDlg : public QDialog
{
	Q_OBJECT

public:
	explicit CPassageNavigatorDlg(CBibleDatabasePtr pBibleDatabase,
									 QWidget *parent,
									 CPassageNavigator::NavigatorRefTypeOptionFlags flagsRefTypes = CPassageNavigator::NRTO_Default,
									 CPassageNavigator::NAVIGATOR_REF_TYPE_ENUM nRefType = CPassageNavigator::NRTE_WORD);
	virtual ~CPassageNavigatorDlg();

	void setGotoButtonText(const QString &strText);

	TPhraseTag passage() const;
	void setPassage(const TPhraseTag &tag);
	CPassageNavigator &navigator();

	CPassageNavigator::NAVIGATOR_REF_TYPE_ENUM refType() const;
	void setRefType(CPassageNavigator::NAVIGATOR_REF_TYPE_ENUM nRefType);

signals:
	void gotoIndex(const TPhraseTag &tag);

public slots:
	virtual void accept() override;

private slots:
	void en_modeChanged(bool bRelative);
	void en_ApplyResolvedClicked();
	void en_ModeClicked();
	void en_gotoIndex(const TPhraseTag &tag);
	void en_resizeMe();

private:
	CBibleDatabasePtr m_pBibleDatabase;
	QPushButton *m_pApplyButton;		// Apply is to apply the resolved reference to the start reference for relative mode
	QPushButton *m_pModeButton;			// Mode button is to switch Relative/Absolute modes
	QPushButton *m_pResetButton;		// Clear passage offset
	QPushButton *m_pOKButton;			// Goto passage button
	QPushButton *m_pCancelButton;		// Abort
	CPassageNavigator *m_pNavigator;
	Ui::CPassageNavigatorDlg ui;
};

// ============================================================================

// SmartPointer classes needed, particularly for stack instantiated dialogs, since
//		this dialog is only WindowModal and the parent can get deleted during an
//		app close event, causing an attempted double-free which leads to a crash:
class CPassageNavigatorDlgPtr : public QPointer<CPassageNavigatorDlg>
{
public:
	CPassageNavigatorDlgPtr(CBibleDatabasePtr pBibleDatabase,
								QWidget *parent = nullptr,
								CPassageNavigator::NavigatorRefTypeOptionFlags flagsRefTypes = CPassageNavigator::NRTO_Default,
								CPassageNavigator::NAVIGATOR_REF_TYPE_ENUM nRefType = CPassageNavigator::NRTE_WORD)
		:	QPointer<CPassageNavigatorDlg>(new CPassageNavigatorDlg(pBibleDatabase, parent, flagsRefTypes, nRefType))
	{

	}

	virtual ~CPassageNavigatorDlgPtr()
	{
		if (!isNull()) delete data();
	}
};

// ============================================================================

#endif // PASSAGE_NAVIGATOR_DLG_H
