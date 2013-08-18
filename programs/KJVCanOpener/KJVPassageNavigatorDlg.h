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

#ifndef KJVPASSAGENAVIGATORDLG_H
#define KJVPASSAGENAVIGATORDLG_H

#include "dbstruct.h"
#include "KJVPassageNavigator.h"

#include <QDialog>
#include <QAbstractButton>
#include <QPushButton>

namespace Ui {
class CKJVPassageNavigatorDlg;
}

class CKJVPassageNavigatorDlg : public QDialog
{
	Q_OBJECT

public:
	explicit CKJVPassageNavigatorDlg(CBibleDatabasePtr pBibleDatabase,
									 QWidget *parent,
									 CKJVPassageNavigator::NavigatorRefTypeOptionFlags flagsRefTypes = CKJVPassageNavigator::NRTO_Default,
									 CKJVPassageNavigator::NAVIGATOR_REF_TYPE_ENUM nRefType = CKJVPassageNavigator::NRTE_WORD);
	virtual ~CKJVPassageNavigatorDlg();

	void setGotoButtonText(const QString &strText);

	TPhraseTag passage() const;
	void setPassage(const TPhraseTag &tag);
	CKJVPassageNavigator &navigator();

	CKJVPassageNavigator::NAVIGATOR_REF_TYPE_ENUM refType() const;
	void setRefType(CKJVPassageNavigator::NAVIGATOR_REF_TYPE_ENUM nRefType);

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
	CKJVPassageNavigator *m_pNavigator;
	Ui::CKJVPassageNavigatorDlg *ui;
};

#endif // KJVPASSAGENAVIGATORDLG_H
