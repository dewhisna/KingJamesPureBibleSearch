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
										bool bRemoveColophons,
										bool bRemoveSuperscriptions,
										bool bWordsOfJesusOnly,
										bool bIncludeBookPrologues,
										QWidget *parent = nullptr);
	~CELSBibleDatabaseSelectDlg();

	QString bibleUUID() const { return m_strBibleUUID; }
	// ----
	bool removeColophons() const { return m_bRemoveColophons; }
	// ----
	bool removeSuperscriptions() const { return m_bRemoveSuperscriptions; }
	// ----
	bool wordsOfJesusOnly() const { return m_bWordsOfJesusOnly; }
	// ----
	bool includeBookPrologues() const { return m_bIncludeBookPrologues; }

public slots:
	void setBibleUUID(const QString &strBibleUUID) { m_strBibleUUID = strBibleUUID; }
	void setRemoveColophons(bool bRemoveColophons) { m_bRemoveColophons = bRemoveColophons; }
	void setRemoveSuperscriptions(bool bRemoveSuperscriptions) { m_bRemoveSuperscriptions = bRemoveSuperscriptions; }
	void setWordsOfJesusOnly(bool bWordsOfJesusOnly) { m_bWordsOfJesusOnly = bWordsOfJesusOnly; }
	void setIncludeBookPrologues(bool bIncludeBookPrologues) { m_bIncludeBookPrologues = bIncludeBookPrologues; }

private slots:
	void en_selectionChanged(int nIndex);

private:
	Ui::CELSBibleDatabaseSelectDlg *ui;
	// ----
	QString m_strBibleUUID;
	bool m_bRemoveColophons = false;
	bool m_bRemoveSuperscriptions = false;
	bool m_bWordsOfJesusOnly = false;
	bool m_bIncludeBookPrologues = false;
};

// ============================================================================

#endif // ELSBIBLEDATABASESELECTDLG_H
