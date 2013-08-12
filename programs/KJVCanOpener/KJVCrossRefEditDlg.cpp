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

#include "UserNotesDatabase.h"

#include "KJVSearchResult.h"
#include "ScriptureEdit.h"
#include "PhraseEdit.h"

#include <QMessageBox>

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

CKJVCrossRefEditDlg::CKJVCrossRefEditDlg(CBibleDatabasePtr pBibleDatabase, QWidget *parent)
	:	QDialog(parent),
		m_pBibleDatabase(pBibleDatabase),
		ui(new Ui::CKJVCrossRefEditDlg),
		m_pEditSourcePassage(NULL),
		m_pCrossRefTreeView(NULL),
		m_bIsDirty(false),
		m_bHaveGeometry(false)
{
	assert(m_pBibleDatabase != NULL);

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

	m_pCrossRefTreeView = new CSearchResultsTreeView(m_pBibleDatabase, this);
	m_pCrossRefTreeView->setObjectName("treeViewCrossRefs");
	QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Expanding);
	sizePolicy2.setHorizontalStretch(5);
	sizePolicy2.setVerticalStretch(0);
	sizePolicy2.setHeightForWidth(m_pCrossRefTreeView->sizePolicy().hasHeightForWidth());
	m_pCrossRefTreeView->setSizePolicy(sizePolicy2);
	m_pCrossRefTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
	m_pCrossRefTreeView->setToolTip(tr("Cross Reference Passages Linked to the Source Reference"));

	delete ui->treeCrossRefs;
	ui->treeCrossRefs = NULL;
	ui->verticalLayoutRefList->insertWidget(ndx, m_pCrossRefTreeView);

	connect(m_pCrossRefTreeView, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(en_crossRefTreeViewContextMenuRequested(const QPoint &)));

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

void CKJVCrossRefEditDlg::setSourcePassage(const TPhraseTag &tag)
{
	m_tagSourcePassage = tag;
	CRelIndex ndxRel = m_tagSourcePassage.relIndex();
	ndxRel.setWord(0);			// Make sure we have only a book, chapter, or verse

	ui->editSourceRefDesc->setText(m_pBibleDatabase->PassageReferenceText(ndxRel));

	if (ndxRel.verse()) {
		m_pEditSourcePassage->navigator().setDocumentToVerse(ndxRel, CPhraseNavigator::TRO_NoAnchors | CPhraseNavigator::TRO_AllUserNotesVisible);
	} else if (ndxRel.chapter()) {
		m_pEditSourcePassage->navigator().setDocumentToChapter(ndxRel, defaultDocumentToChapterFlags | CPhraseNavigator::TRO_NoAnchors | CPhraseNavigator::TRO_AllUserNotesVisible);
	} else {
		m_pEditSourcePassage->navigator().setDocumentToBookInfo(ndxRel, CPhraseNavigator::TRO_NoAnchors | CPhraseNavigator::TRO_AllUserNotesVisible);
	}

}

// ============================================================================
void CKJVCrossRefEditDlg::accept()
{
	assert(g_pUserNotesDatabase != NULL);

	// TODO : Set Cross References in User Notes Database

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

// ============================================================================

void CKJVCrossRefEditDlg::en_AddReferenceClicked()
{
	// TODO : Finish
}

void CKJVCrossRefEditDlg::en_DelReferenceClicked()
{
	// TODO : Finish
}

// ============================================================================
