/****************************************************************************
**
** Copyright (C) 2013-2025 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef CROSS_REF_EDIT_DLG_H
#define CROSS_REF_EDIT_DLG_H

#include "dbstruct.h"
#include "UserNotesDatabase.h"

#include <QDialog>
#include <QPoint>
#include <QSettings>
#include <QPointer>

// ============================================================================

// Forward Declarations:
class CScriptureEdit;
class CSearchResultsTreeView;

// ============================================================================

#include "ui_CrossRefEditDlg.h"

class CCrossRefEditDlg : public QDialog
{
	Q_OBJECT
	
public:
	explicit CCrossRefEditDlg(CBibleDatabasePtr pBibleDatabase, CUserNotesDatabasePtr pUserNotesDatabase, QWidget *parent = nullptr);
	virtual ~CCrossRefEditDlg();

	virtual void writeSettings(QSettings &settings, const QString &prefix = "CrossRefsEditor");
	virtual void readSettings(QSettings &settings, const QString &prefix = "CrossRefsEditor");

	TPassageTag sourcePassage() const { return m_tagSourcePassage; }
	void setSourcePassage(const TPassageTag &tag);

public slots:
	virtual void accept() override;
	virtual void reject() override;

private slots:
	void saveCrossRefs();
	void en_crossRefTreeViewContextMenuRequested(const QPoint &pos);
	void en_crossRefTreeViewCurrentItemChanged();
	void en_crossRefTreeViewSelectionListChanged();
	void en_crossRefTreeViewEntryActivated(const QModelIndex &index);		// Enter or double-click activated
	void en_SelectSourceReferenceClicked();
	void en_AddReferenceClicked();
	void en_DelReferenceClicked();

private:
	CRelIndex navigateCrossRef(const CRelIndex &ndxStart);		// Bring up navigator at specified starting location for entering a cross-ref and return selected ref.  If user cancels, returns CRelIndex()

// Data Private:
private:
	CBibleDatabasePtr m_pBibleDatabase;
	CUserNotesDatabasePtr m_pUserNotesDatabase;
	TPassageTag m_tagSourcePassage;

	CUserNotesDatabasePtr m_pWorkingUserNotesDatabase;			// Working user notes allows us to model cross-refs in the tree-view without nuking the main database until the user accepts changes

// UI Private:
private:
	CScriptureEdit *m_pEditSourcePassage;
	CSearchResultsTreeView *m_pCrossRefTreeView;			// Tree View holding our Cross-References List
	// ----
	bool m_bIsDirty;
	bool m_bHaveGeometry;
	// ----
	Ui::CCrossRefEditDlg ui;
};

// ============================================================================

// SmartPointer classes needed, particularly for stack instantiated dialogs, since
//		this dialog is only WindowModal and the parent can get deleted during an
//		app close event, causing an attempted double-free which leads to a crash:
class CCrossRefEditDlgPtr : public QPointer<CCrossRefEditDlg>
{
public:
	CCrossRefEditDlgPtr(CBibleDatabasePtr pBibleDatabase, CUserNotesDatabasePtr pUserNotesDatabase, QWidget *parent = nullptr)
		:	QPointer<CCrossRefEditDlg>(new CCrossRefEditDlg(pBibleDatabase, pUserNotesDatabase, parent))
	{

	}

	virtual ~CCrossRefEditDlgPtr()
	{
		if (!isNull()) delete data();
	}
};

// ============================================================================

#endif // CROSS_REF_EDIT_DLG_H
