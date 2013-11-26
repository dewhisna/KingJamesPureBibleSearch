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

#include "KJVNoteEditDlg.h"

#include "main.h"
#include "PersistentSettings.h"

#include "KJVPassageNavigatorDlg.h"
#include "ScriptureDocument.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QByteArray>
#include <QMessageBox>
#include <QIcon>
#include <QPair>
#include <QTextDocument>
#include <QTextCursor>
#include <QToolTip>
#include <QPoint>

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

CKJVNoteEditDlg::CKJVNoteEditDlg(CBibleDatabasePtr pBibleDatabase, CUserNotesDatabasePtr pUserNotesDatabase, QWidget *parent)
	:	QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint),
		m_pBackgroundColorButton(NULL),
		m_pRichTextEdit(NULL),
		m_pDeleteNoteButton(NULL),
		m_pBibleDatabase(pBibleDatabase),
		m_pUserNotesDatabase(pUserNotesDatabase),
		m_bDoingUpdate(false),
		m_bIsDirty(false),
		m_bHaveGeometry(false)
{
	assert(pBibleDatabase.data() != NULL);
	assert(pUserNotesDatabase.data() != NULL);

	ui.setupUi(this);

	// --------------------------------------------------------------

	int ndx;
	int nRow;
	int nCol;
	int nRowSpan;
	int nColSpan;

	// --------------------------------------------------------------

	//	Swapout the textEdit from the layout with a QwwRichTextEdit:

	ndx = ui.gridLayoutMain->indexOf(ui.textEdit);
	assert(ndx != -1);
	if (ndx == -1) return;
	ui.gridLayoutMain->getItemPosition(ndx, &nRow, &nCol, &nRowSpan, &nColSpan);

	m_pRichTextEdit = new QwwRichTextEdit(this);
	m_pRichTextEdit->setObjectName(QString::fromUtf8("textEdit"));
	m_pRichTextEdit->setMouseTracking(true);
//	m_pRichTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
//	m_pRichTextEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_pRichTextEdit->setSizePolicy(ui.textEdit->sizePolicy());
	m_pRichTextEdit->setTabChangesFocus(false);
	m_pRichTextEdit->setTextInteractionFlags(Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse|Qt::TextEditable);

	delete ui.textEdit;
	ui.textEdit = NULL;
	ui.gridLayoutMain->addWidget(m_pRichTextEdit, nRow, nCol, nRowSpan, nColSpan);

	// --------------------------------------------------------------

	//	Swapout the buttonBackgroundColor from the layout with a QwwColorButton:

	ndx = ui.verticalLayoutBackgroundButtons->indexOf(ui.buttonBackgroundColor);
	assert(ndx != -1);
	if (ndx == -1) return;

	m_pBackgroundColorButton = new QwwColorButton(this);
	m_pBackgroundColorButton->setObjectName(QString::fromUtf8("buttonBackgroundColor"));
	m_pBackgroundColorButton->setShowName(false);			// Must do this before setting our real text
	m_pBackgroundColorButton->setText(tr("Note Background Color"));
	m_pBackgroundColorButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

	delete ui.buttonBackgroundColor;
	ui.buttonBackgroundColor = NULL;
	ui.verticalLayoutBackgroundButtons->insertWidget(ndx, m_pBackgroundColorButton);

	// --------------------------------------------------------------

	// Reinsert it in the correct TabOrder:
	QWidget::setTabOrder(m_pRichTextEdit, ui.buttonBox);
	QWidget::setTabOrder(ui.buttonBox, ui.editNoteLocation);
	QWidget::setTabOrder(ui.editNoteLocation, m_pBackgroundColorButton);
	QWidget::setTabOrder(m_pBackgroundColorButton, ui.buttonSetAsDefaultBackgroundColor);
	QWidget::setTabOrder(ui.buttonSetAsDefaultBackgroundColor, ui.widgetNoteKeywords);
	QWidget::setTabOrder(ui.widgetNoteKeywords, ui.buttonInsertLink);

	// --------------------------------------------------------------

	// Setup Dialog buttons:

	assert(ui.buttonBox->button(QDialogButtonBox::Ok) != NULL);
	ui.buttonBox->button(QDialogButtonBox::Ok)->setIcon(QIcon(":/res/ok_blue-24.png"));
	assert(ui.buttonBox->button(QDialogButtonBox::Cancel) != NULL);
	ui.buttonBox->button(QDialogButtonBox::Cancel)->setIcon(QIcon(":/res/cancel-24.png"));
	m_pDeleteNoteButton = ui.buttonBox->addButton(tr("Delete Note"), QDialogButtonBox::ActionRole);
	m_pDeleteNoteButton->setIcon(QIcon(":/res/deletered1-24.png"));
	connect(ui.buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(en_ButtonClicked(QAbstractButton*)));

	// --------------------------------------------------------------

	ui.widgetNoteKeywords->setMode(KWME_EDITOR);

	// --------------------------------------------------------------

	connect(m_pRichTextEdit, SIGNAL(textChanged()), this, SLOT(en_textChanged()));
	connect(m_pBackgroundColorButton, SIGNAL(colorPicked(const QColor &)), this, SLOT(en_BackgroundColorPicked(const QColor &)));
	connect(ui.buttonSetAsDefaultBackgroundColor, SIGNAL(clicked()), this, SLOT(en_setDefaultNoteBackgroundColor()));
	connect(ui.widgetNoteKeywords, SIGNAL(keywordListChanged()), this, SLOT(en_keywordListChanged()));
	connect(ui.buttonInsertLink, SIGNAL(clicked()), this, SLOT(en_clickedInsertReferenceLink()));

	m_pRichTextEdit->setFocus();

	// --------------------------------------------------------------

#ifndef Q_OS_MAC
	setWindowModality(Qt::WindowModal);		// Only block our parentCanOpener, not the whole app
#endif
}

CKJVNoteEditDlg::~CKJVNoteEditDlg()
{

}

// ============================================================================

void CKJVNoteEditDlg::writeSettings(QSettings &settings, const QString &prefix)
{
	// RestoreState:
	settings.beginGroup(groupCombine(prefix, constrRestoreStateGroup));
	if (m_bHaveGeometry) settings.setValue(constrGeometryKey, saveGeometry());
	settings.endGroup();
}

void CKJVNoteEditDlg::readSettings(QSettings &settings, const QString &prefix)
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

void CKJVNoteEditDlg::setLocationIndex(const CRelIndex &ndxLocation)
{
	assert(m_pRichTextEdit != NULL);
	assert(m_pUserNotesDatabase.data() != NULL);

	m_ndxLocation = ndxLocation;
	m_ndxLocation.setWord(0);		// Work with whole verses only
	ui.editNoteLocation->setText(m_pBibleDatabase->PassageReferenceText(m_ndxLocation));

	m_UserNote = m_pUserNotesDatabase->noteFor(m_ndxLocation);

	m_bDoingUpdate = true;

	m_pBackgroundColorButton->setCurrentColor(m_UserNote.backgroundColor());
	setBackgroundColorPreview();

	// Setup Keywords:
	ui.widgetNoteKeywords->setKeywordList(m_UserNote.keywordList(), m_pUserNotesDatabase->compositeKeywordList());

	m_pRichTextEdit->setHtml(m_UserNote.text());
	m_bIsDirty = false;

	m_bDoingUpdate = false;
}

// ============================================================================

void CKJVNoteEditDlg::accept()
{
	assert(m_pRichTextEdit != NULL);
	assert(m_pUserNotesDatabase.data() != NULL);

	if (ui.widgetNoteKeywords->haveUnenteredKeywords()) {
		int nResult = QMessageBox::warning(this, windowTitle(), tr("It appears you have typed some keyword text, but "
																   "haven't yet entered them to where they will take effect.\n\n"
																   "Do you wish to set them as valid keywords for this note?"),
																	(QMessageBox::Yes | QMessageBox::No), QMessageBox::Yes);
		if (nResult == QMessageBox::Yes) ui.widgetNoteKeywords->enterKeywords();
	}

	m_UserNote.setText(m_pRichTextEdit->toHtml());
	m_UserNote.setIsVisible(true);			// Make note visible when they are explicitly setting it
	m_pUserNotesDatabase->setNoteFor(m_ndxLocation, m_UserNote);
	m_bIsDirty = false;
	m_bHaveGeometry = true;
	QDialog::accept();
}

void CKJVNoteEditDlg::reject()
{
	if (m_bIsDirty) {
		int nResult = QMessageBox::warning(this, windowTitle(), tr("You have made changes to this note.  Do you wish to discard them??"),
																	(QMessageBox::Ok | QMessageBox::Cancel), QMessageBox::Cancel);
		if (nResult != QMessageBox::Ok) return;
	}

	m_bHaveGeometry = true;
	QDialog::reject();
}

// ============================================================================

void CKJVNoteEditDlg::en_textChanged()
{
	if (m_bDoingUpdate) return;

	m_bIsDirty = true;
}

// ============================================================================

void CKJVNoteEditDlg::setBackgroundColorPreview()
{
	setStyleSheet(QString("QwwRichTextEdit { background-color:%1; }\n").arg(m_UserNote.backgroundColor().name()));
}

void CKJVNoteEditDlg::en_BackgroundColorPicked(const QColor &color)
{
	if (m_bDoingUpdate) return;
	m_bDoingUpdate = true;

	m_UserNote.setBackgroundColor(color);
	setBackgroundColorPreview();
	m_bIsDirty = true;

	m_bDoingUpdate = false;
}

void CKJVNoteEditDlg::en_setDefaultNoteBackgroundColor()
{
	CPersistentSettings::instance()->setColorDefaultNoteBackground(m_UserNote.backgroundColor());
	QPoint ptPos = pos();
	ptPos.setX(ptPos.x() + (size().width() / 2));
	ptPos.setY(ptPos.y() + (size().height() / 2));
	QToolTip::showText(ptPos, tr("Default Note Background Color Has Been Set"), this);
}

// ============================================================================

void CKJVNoteEditDlg::en_ButtonClicked(QAbstractButton *button)
{
	assert(button != NULL);
	assert(m_pUserNotesDatabase.data() != NULL);

	if (button == m_pDeleteNoteButton) {
		int nResult = QMessageBox::warning(this, windowTitle(), tr("Are you sure you want to completely delete this note??"),
																	(QMessageBox::Ok | QMessageBox::Cancel), QMessageBox::Cancel);
		if (nResult != QMessageBox::Ok) return;
		m_bIsDirty = false;
		m_pUserNotesDatabase->removeNoteFor(m_ndxLocation);
		QDialog::accept();
	}
}

// ============================================================================

void CKJVNoteEditDlg::en_keywordListChanged()
{
	if (m_bDoingUpdate) return;
	m_bDoingUpdate = true;

	m_UserNote.setKeywordList(ui.widgetNoteKeywords->selectedKeywordList());
	m_bIsDirty = true;

	m_bDoingUpdate = false;
}

// ============================================================================

void CKJVNoteEditDlg::en_clickedInsertReferenceLink()
{
	CRelIndex ndxStart = m_ndxLastRefLink;

	if (!ndxStart.isSet()) ndxStart = m_ndxLocation;

	CRelIndex ndxTarget = navigateCrossRef(ndxStart);

	if (ndxTarget.isSet()) {
		m_ndxLastRefLink = ndxTarget;

		CScriptureTextHtmlBuilder refHTML;
		refHTML.addRefLinkFor(m_pBibleDatabase.data(), ndxTarget, true);

		m_pRichTextEdit->insertHtml(refHTML.getResult());
		m_bIsDirty = true;
	}
}

CRelIndex CKJVNoteEditDlg::navigateCrossRef(const CRelIndex &ndxStart)
{
	CKJVPassageNavigator::NAVIGATOR_REF_TYPE_ENUM nType = CKJVPassageNavigator::NRTE_VERSE;
	if (ndxStart.verse() == 0) nType = CKJVPassageNavigator::NRTE_CHAPTER;
	if (ndxStart.chapter() == 0) nType = CKJVPassageNavigator::NRTE_BOOK;

	CKJVPassageNavigatorDlgPtr pDlg(m_pBibleDatabase, this, CKJVPassageNavigator::NRTO_Verse | CKJVPassageNavigator::NRTO_Chapter | CKJVPassageNavigator::NRTO_Book, nType);
	pDlg->setGotoButtonText(tr("&OK"));
	TPhraseTag tagNav(ndxStart);
	pDlg->navigator().startAbsoluteMode(tagNav);
	if (pDlg->exec() != QDialog::Accepted) return CRelIndex();

	if (pDlg != NULL) {			// Could get deleted during execution
		CRelIndex ndxTarget = pDlg->passage().relIndex();
		ndxTarget.setWord(0);			// Whole verse references only
		return ndxTarget;
	}
	return CRelIndex();
}

// ============================================================================
