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

#include "KJVCrossRefEditDlg.h"
#include "ui_KJVCrossRefEditDlg.h"

#include "PersistentSettings.h"

#include "KJVSearchResult.h"
#include "ScriptureEdit.h"
#include "PhraseEdit.h"
#include "KJVPassageNavigatorDlg.h"

#include <QMessageBox>
#include <QTextCursor>

#include <assert.h>

// ============================================================================

namespace {
	//////////////////////////////////////////////////////////////////////
	// File-scoped constants
	//////////////////////////////////////////////////////////////////////

	// Key constants:
	// --------------

	// RestoreState:
	const QString constrRestoreStateGroup("RestoreState");
	const QString constrGeometryKey("Geometry");
	const QString constrWindowStateKey("WindowState");

}

// ============================================================================

QAction *CKJVCrossRefEditDlg::m_pActionCrossRefsEditor = NULL;

QAction *CKJVCrossRefEditDlg::actionCrossRefsEditor()
{
	assert(m_pActionCrossRefsEditor != NULL);
	return m_pActionCrossRefsEditor;
}

void CKJVCrossRefEditDlg::setActionCrossRefsEditor(QAction *pAction)
{
	assert(m_pActionCrossRefsEditor == NULL);
	m_pActionCrossRefsEditor = pAction;
}

// ============================================================================

CKJVCrossRefEditDlg::CKJVCrossRefEditDlg(CBibleDatabasePtr pBibleDatabase, CUserNotesDatabasePtr pUserNotesDatabase, QWidget *parent)
	:	QDialog(parent),
		m_pBibleDatabase(pBibleDatabase),
		m_pUserNotesDatabase(pUserNotesDatabase),
		ui(new Ui::CKJVCrossRefEditDlg),
		m_pEditSourcePassage(NULL),
		m_pCrossRefTreeView(NULL),
		m_bIsDirty(false),
		m_bHaveGeometry(false)
{
	assert(m_pBibleDatabase != NULL);
	assert(m_pUserNotesDatabase != NULL);

	// Create a working copy and initialize it to the existing database:
	m_pWorkingUserNotesDatabase = QSharedPointer<CUserNotesDatabase>(new CUserNotesDatabase());
//	m_pWorkingUserNotesDatabase->setDataFrom(*(m_pUserNotesDatabase.data()));
	m_pWorkingUserNotesDatabase->setCrossRefsMap(m_pUserNotesDatabase->crossRefsMap());

	ui->setupUi(this);

	int ndx;

	// --------------------------------------------------------------

	//	Swapout the editSourcePassage from the layout with
	//		one that we can set the database on:

	ndx = ui->verticalLayoutSource->indexOf(ui->editSourcePassage);
	assert(ndx != -1);
	if (ndx == -1) return;

	m_pEditSourcePassage = new CScriptureEdit(m_pBibleDatabase, this);
	m_pEditSourcePassage->setObjectName("editSourcePassage");
	m_pEditSourcePassage->setMinimumSize(QSize(200, 150));
	QSizePolicy sizePolicy1(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	sizePolicy1.setHorizontalStretch(10);
	sizePolicy1.setVerticalStretch(0);
	sizePolicy1.setHeightForWidth(m_pEditSourcePassage->sizePolicy().hasHeightForWidth());
	m_pEditSourcePassage->setSizePolicy(sizePolicy1);
	m_pEditSourcePassage->setMouseTracking(true);
	m_pEditSourcePassage->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	m_pEditSourcePassage->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_pEditSourcePassage->setAcceptDrops(false);
	m_pEditSourcePassage->setUndoRedoEnabled(false);
	m_pEditSourcePassage->setTabChangesFocus(true);
	m_pEditSourcePassage->setTextInteractionFlags(Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);
	m_pEditSourcePassage->setContextMenuPolicy(Qt::NoContextMenu);
	m_pEditSourcePassage->setToolTip(tr("Source Passage for Reference"));

	delete ui->editSourcePassage;
	ui->editSourcePassage = NULL;
	ui->verticalLayoutSource->insertWidget(ndx, m_pEditSourcePassage);


	// --------------------------------------------------------------

	//	Swapout the treeCrossRefs from the layout with
	//		one that we can set the database on:

	ndx = ui->verticalLayoutRefList->indexOf(ui->treeCrossRefs);
	assert(ndx != -1);
	if (ndx == -1) return;

	m_pCrossRefTreeView = new CSearchResultsTreeView(m_pBibleDatabase, m_pWorkingUserNotesDatabase, this);
	m_pCrossRefTreeView->setObjectName("treeViewCrossRefs");
	QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Expanding);
	sizePolicy2.setHorizontalStretch(5);
	sizePolicy2.setVerticalStretch(0);
	sizePolicy2.setHeightForWidth(m_pCrossRefTreeView->sizePolicy().hasHeightForWidth());
	m_pCrossRefTreeView->setSizePolicy(sizePolicy2);
	m_pCrossRefTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
	m_pCrossRefTreeView->setToolTip(tr("Cross Reference Passages Linked to the Source Reference"));
	m_pCrossRefTreeView->setViewMode(CVerseListModel::VVME_CROSSREFS);
	m_pCrossRefTreeView->setDisplayMode(CVerseListModel::VDME_RICHTEXT);

	delete ui->treeCrossRefs;
	ui->treeCrossRefs = NULL;
	ui->verticalLayoutRefList->insertWidget(ndx, m_pCrossRefTreeView);

	connect(m_pCrossRefTreeView, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(en_crossRefTreeViewContextMenuRequested(const QPoint &)));
	connect(m_pCrossRefTreeView, SIGNAL(currentItemChanged()), this, SLOT(en_crossRefTreeViewCurrentItemChanged()));
	connect(m_pCrossRefTreeView, SIGNAL(selectionListChanged()), this, SLOT(en_crossRefTreeViewSelectionListChanged()));
	connect(m_pCrossRefTreeView, SIGNAL(activated(const QModelIndex &)), this, SLOT(en_crossRefTreeViewEntryActivated(const QModelIndex &)));

	// --------------------------------------------------------------

	connect(ui->buttonAddRef, SIGNAL(clicked()), this, SLOT(en_AddReferenceClicked()));
	connect(ui->buttonDeleteRef, SIGNAL(clicked()), this, SLOT(en_DelReferenceClicked()));

	// --------------------------------------------------------------

}

CKJVCrossRefEditDlg::~CKJVCrossRefEditDlg()
{
	delete ui;
}

// ============================================================================

void CKJVCrossRefEditDlg::writeSettings(QSettings &settings, const QString &prefix)
{
	// RestoreState:
	settings.beginGroup(groupCombine(prefix, constrRestoreStateGroup));
	if (m_bHaveGeometry) settings.setValue(constrGeometryKey, saveGeometry());
	settings.endGroup();
}

void CKJVCrossRefEditDlg::readSettings(QSettings &settings, const QString &prefix)
{
	// RestoreState:
	settings.beginGroup(groupCombine(prefix, constrRestoreStateGroup));
	if (!settings.value(constrGeometryKey).toByteArray().isEmpty()) {
		restoreGeometry(settings.value(constrGeometryKey).toByteArray());
		m_bHaveGeometry = true;
	}
	settings.endGroup();
}

// ============================================================================

void CKJVCrossRefEditDlg::setSourcePassage(const TPassageTag &tag)
{
	CRelIndex ndxRel = tag.relIndex();
	ndxRel.setWord(0);			// Make sure we have only a book, chapter, or verse
	m_tagSourcePassage = TPassageTag(ndxRel, tag.verseCount());

	ui->editSourceRefDesc->setText(m_pBibleDatabase->PassageReferenceText(ndxRel));

	if (ndxRel.verse()) {
		m_pEditSourcePassage->navigator().setDocumentToVerse(ndxRel, CPhraseNavigator::TRO_NoAnchors | CPhraseNavigator::TRO_AllUserNotesVisible);
	} else if (ndxRel.chapter()) {
		m_pEditSourcePassage->navigator().setDocumentToChapter(ndxRel, defaultDocumentToChapterFlags | CPhraseNavigator::TRO_NoAnchors | CPhraseNavigator::TRO_AllUserNotesVisible | CPhraseNavigator::TRO_SuppressPrePostChapters);
		QTextCursor txtCursor;
		txtCursor = m_pEditSourcePassage->textCursor();
		txtCursor.movePosition(QTextCursor::Start);
		m_pEditSourcePassage->setTextCursor(txtCursor);
	} else {
		m_pEditSourcePassage->navigator().setDocumentToBookInfo(ndxRel, defaultDocumentToBookInfoFlags | CPhraseNavigator::TRO_NoAnchors | CPhraseNavigator::TRO_AllUserNotesVisible);
	}

	// Update working database from source database:
//	m_pWorkingUserNotesDatabase->setDataFrom(*(m_pUserNotesDatabase.data()));
	m_pWorkingUserNotesDatabase->setCrossRefsMap(m_pUserNotesDatabase->crossRefsMap());
	m_pCrossRefTreeView->setSingleCrossRefSourceIndex(ndxRel);
	m_bIsDirty = false;
}

// ============================================================================
void CKJVCrossRefEditDlg::accept()
{
	assert(m_pUserNotesDatabase != NULL);

	m_pUserNotesDatabase->setCrossRefsMap(m_pWorkingUserNotesDatabase->crossRefsMap());

	m_bIsDirty = false;
	m_bHaveGeometry = true;
	QDialog::accept();
}

void CKJVCrossRefEditDlg::reject()
{
	if (m_bIsDirty) {
		int nResult = QMessageBox::warning(this, windowTitle(), tr("You have made changes to this Cross Reference.  Do you wish to discard them??"),
																	(QMessageBox::Ok | QMessageBox::Cancel), QMessageBox::Cancel);
		if (nResult != QMessageBox::Ok) return;
	}

	m_bHaveGeometry = true;
	QDialog::reject();
}

// ============================================================================

void CKJVCrossRefEditDlg::en_crossRefTreeViewContextMenuRequested(const QPoint &pos)
{
	// TODO : Finish
}

void CKJVCrossRefEditDlg::en_crossRefTreeViewCurrentItemChanged()
{

}

void CKJVCrossRefEditDlg::en_crossRefTreeViewSelectionListChanged()
{
	QModelIndexList lstSelectedItems = m_pCrossRefTreeView->selectionModel()->selectedRows();
	ui->buttonDeleteRef->setEnabled(lstSelectedItems.size() != 0);
}

void CKJVCrossRefEditDlg::en_crossRefTreeViewEntryActivated(const QModelIndex &index)
{
	CRelIndex ndxInitial = m_pCrossRefTreeView->vlmodel()->navigationIndexForModelIndex(index);
	assert(ndxInitial.isSet());
	CRelIndex ndxTarget = navigateCrossRef(ndxInitial);
	if ((ndxTarget.isSet()) && (ndxInitial != ndxTarget)) {
		bool bRemove = m_pWorkingUserNotesDatabase->removeCrossReference(m_tagSourcePassage.relIndex(), ndxInitial);
		assert(bRemove);
		bool bAdd = m_pWorkingUserNotesDatabase->setCrossReference(m_tagSourcePassage.relIndex(), ndxTarget);
		assert(bAdd);
		if (bAdd || bRemove) m_bIsDirty = true;
	}
}

// ============================================================================

CRelIndex CKJVCrossRefEditDlg::navigateCrossRef(const CRelIndex &ndxStart)
{
	CKJVPassageNavigator::NAVIGATOR_REF_TYPE_ENUM nType = CKJVPassageNavigator::NRTE_VERSE;
	if (ndxStart.verse() == 0) nType = CKJVPassageNavigator::NRTE_CHAPTER;
	if (ndxStart.chapter() == 0) nType = CKJVPassageNavigator::NRTE_BOOK;

	CKJVPassageNavigatorDlg dlg(m_pBibleDatabase, this, CKJVPassageNavigator::NRTO_Verse | CKJVPassageNavigator::NRTO_Chapter | CKJVPassageNavigator::NRTO_Book, nType);
	dlg.setGotoButtonText(tr("&OK"));
	TPhraseTag tagNav(ndxStart);
	dlg.navigator().startAbsoluteMode(tagNav);
	if (dlg.exec() != QDialog::Accepted) return CRelIndex();

	CRelIndex ndxTarget = dlg.passage().relIndex();
	ndxTarget.setWord(0);			// Whole verse references only
	return ndxTarget;
}

// ============================================================================

void CKJVCrossRefEditDlg::en_AddReferenceClicked()
{
	CRelIndex ndxTarget = navigateCrossRef(m_tagSourcePassage.relIndex());
	if ((ndxTarget.isSet()) && (m_pWorkingUserNotesDatabase->setCrossReference(m_tagSourcePassage.relIndex(), ndxTarget))) {
		m_bIsDirty = true;
	}
}

void CKJVCrossRefEditDlg::en_DelReferenceClicked()
{
	TRelativeIndexList lstRefsToRemove;

	// Fetch our list of selections first because once we start removing them, our selection will be changing:
	QModelIndexList lstSelectedItems = m_pCrossRefTreeView->selectionModel()->selectedRows();
	for (int ndx = 0; ndx < lstSelectedItems.size(); ++ndx) {
		if (lstSelectedItems.at(ndx).isValid()) {
			CRelIndex ndxRel = m_pCrossRefTreeView->vlmodel()->navigationIndexForModelIndex(lstSelectedItems.at(ndx));
			if (ndxRel.isSet()) lstRefsToRemove.push_back(ndxRel);
		}
	}

	bool bSomethingChanged = false;
	for (unsigned int ndx = 0; ndx < lstRefsToRemove.size(); ++ndx) {
		if (m_pWorkingUserNotesDatabase->removeCrossReference(m_tagSourcePassage.relIndex(), lstRefsToRemove.at(ndx))) {
			bSomethingChanged = true;
		}
	}

	if (bSomethingChanged) m_bIsDirty = true;
}

// ============================================================================
