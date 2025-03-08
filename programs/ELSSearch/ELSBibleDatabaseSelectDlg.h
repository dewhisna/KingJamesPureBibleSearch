/****************************************************************************
**
** Copyright (C) 2024-2025 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef ELSBIBLEDATABASESELECTDLG_H
#define ELSBIBLEDATABASESELECTDLG_H

#include "LetterMatrix.h"

#include <QDialog>

// ============================================================================

namespace Ui {
class CELSBibleDatabaseSelectDlg;
}

class CELSBibleDatabaseSelectDlg : public QDialog
{
	Q_OBJECT

public:
	explicit CELSBibleDatabaseSelectDlg(const QString &strBibleUUID,
										LetterMatrixTextModifierOptionFlags flagsLMTMO,
										LMBookPrologueOptionFlags flagsLMBPO,
										LMChapterPrologueOptionFlags flagsLMCPO,
										LMVersePrologueOptionFlags flagsLMVPO,
										QWidget *parent = nullptr);
	~CELSBibleDatabaseSelectDlg();

	QString bibleUUID() const { return m_strBibleUUID; }
	// ----
	LetterMatrixTextModifierOptionFlags textModifierOptions() const { return m_flagsLMTMO; }
	LMBookPrologueOptionFlags bookPrologueOptions() const { return m_flagsLMBPO; }
	LMChapterPrologueOptionFlags chapterPrologueOptions() const { return m_flagsLMCPO; }
	LMVersePrologueOptionFlags versePrologueOptions() const { return m_flagsLMVPO; }

private slots:
	void en_BibleSelectionChanged(int nIndex);
	void en_CPONumberSelectionChanged(int nIndex);
	void en_VPONumberSelectionChanged(int nIndex);

private:
	Ui::CELSBibleDatabaseSelectDlg *ui;
	// ----
	QString m_strBibleUUID;
	LetterMatrixTextModifierOptionFlags m_flagsLMTMO;
	LMBookPrologueOptionFlags m_flagsLMBPO;
	LMChapterPrologueOptionFlags m_flagsLMCPO;
	LMVersePrologueOptionFlags m_flagsLMVPO;
};

// ============================================================================

#endif // ELSBIBLEDATABASESELECTDLG_H
