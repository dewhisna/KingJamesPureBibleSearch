/****************************************************************************
**
** Copyright (C) 2013 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef KJVCROSSREFEDITDLG_H
#define KJVCROSSREFEDITDLG_H

#include "dbstruct.h"

#include <QDialog>
#include <QPoint>
#include <QAction>
#include <QSettings>

// ============================================================================

// Forward Declarations:
class CScriptureEdit;
class CSearchResultsTreeView;

// ============================================================================

namespace Ui {
	class CKJVCrossRefEditDlg;
}

class CKJVCrossRefEditDlg : public QDialog
{
	Q_OBJECT
	
public:
	explicit CKJVCrossRefEditDlg(CBibleDatabasePtr pBibleDatabase, QWidget *parent = 0);
	virtual ~CKJVCrossRefEditDlg();

	virtual void writeSettings(QSettings &settings, const QString &prefix = "CrossRefsEditor");
	virtual void readSettings(QSettings &settings, const QString &prefix = "CrossRefsEditor");

	TPhraseTag sourcePassage() const { return m_tagSourcePassage; }
	void setSourcePassage(const TPhraseTag &tag);

	static QAction *actionCrossRefsEditor();

protected:
	friend class CKJVCanOpener;			// Main App is Friend to create/set initial action
	static void setActionCrossRefsEditor(QAction *pAction);

public slots:
	virtual void accept();
	virtual void reject();

private slots:
	void en_crossRefTreeViewContextMenuRequested(const QPoint &pos);
	void en_AddReferenceClicked();
	void en_DelReferenceClicked();

// Data Private:
private:
	CBibleDatabasePtr m_pBibleDatabase;
	TPhraseTag m_tagSourcePassage;

// UI Private:
private:
	static QAction *m_pActionCrossRefsEditor;
	// ----
	Ui::CKJVCrossRefEditDlg *ui;
	// ----
	CScriptureEdit *m_pEditSourcePassage;
	CSearchResultsTreeView *m_pCrossRefTreeView;			// Tree View holding our Cross-References List
	// ----
	bool m_bIsDirty;
	bool m_bHaveGeometry;
};

// ============================================================================

#endif // KJVCROSSREFEDITDLG_H
