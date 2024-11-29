/****************************************************************************
**
** Copyright (C) 2024 Donna Whisnant, a.k.a. Dewtronics.
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
										QWidget *parent = nullptr);
	~CELSBibleDatabaseSelectDlg();

	QString bibleUUID() const { return m_strBibleUUID; }
	// ----
	LetterMatrixTextModifierOptionFlags textModifierOptions() const { return m_flagsLMTMO; }

private slots:
	void setBibleUUID(const QString &strBibleUUID) { m_strBibleUUID = strBibleUUID; }
	void setRemoveColophons(bool bRemoveColophons) { m_flagsLMTMO.setFlag(LMTMO_RemoveColophons, bRemoveColophons); }
	void setRemoveSuperscriptions(bool bRemoveSuperscriptions) { m_flagsLMTMO.setFlag(LMTMO_RemoveSuperscriptions, bRemoveSuperscriptions); }
	void setWordsOfJesusOnly(bool bWordsOfJesusOnly) { m_flagsLMTMO.setFlag(LMTMO_WordsOfJesusOnly, bWordsOfJesusOnly); }
	void setIncludeBookPrologues(bool bIncludeBookPrologues) { m_flagsLMTMO.setFlag(LMTMO_IncludeBookPrologues, bIncludeBookPrologues); }
	void setIncludeChapterPrologues(bool bIncludeChapterPrologues) { m_flagsLMTMO.setFlag(LMTMO_IncludeChapterPrologues, bIncludeChapterPrologues); }

	void en_selectionChanged(int nIndex);

private:
	Ui::CELSBibleDatabaseSelectDlg *ui;
	// ----
	QString m_strBibleUUID;
	LetterMatrixTextModifierOptionFlags m_flagsLMTMO;
};

// ============================================================================

#endif // ELSBIBLEDATABASESELECTDLG_H
