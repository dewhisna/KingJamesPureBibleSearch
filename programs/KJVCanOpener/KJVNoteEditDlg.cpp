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
#include "ui_KJVNoteEditDlg.h"

#include "PersistentSettings.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QByteArray>
#include <QMessageBox>
#include <QIcon>
#include <QPair>

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

QAction *CKJVNoteEditDlg::m_pActionUserNoteEditor = NULL;

QAction *CKJVNoteEditDlg::actionUserNoteEditor()
{
	assert(m_pActionUserNoteEditor != NULL);
	return m_pActionUserNoteEditor;
}

void CKJVNoteEditDlg::setActionUserNoteEditor(QAction *pAction)
{
	assert(m_pActionUserNoteEditor == NULL);
	m_pActionUserNoteEditor = pAction;
}

// ============================================================================

CKJVNoteEditDlg::CKJVNoteEditDlg(CBibleDatabasePtr pBibleDatabase, QWidget *parent)
	:	QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint),
		ui(new Ui::CKJVNoteEditDlg),
		m_pBackgroundColorButton(NULL),
		m_pRichTextEdit(NULL),
		m_pDeleteNoteButton(NULL),
		m_pBibleDatabase(pBibleDatabase),
		m_bDoingUpdate(false),
		m_bIsDirty(false)
{
	assert(pBibleDatabase != NULL);

	ui->setupUi(this);

	// --------------------------------------------------------------

	int ndx;
	int nRow;
	int nCol;
	int nRowSpan;
	int nColSpan;

	// --------------------------------------------------------------

	//	Swapout the textEdit from the layout with a QwwRichTextEdit:

	ndx = ui->gridLayoutMain->indexOf(ui->textEdit);
	assert(ndx != -1);
	if (ndx == -1) return;
	ui->gridLayoutMain->getItemPosition(ndx, &nRow, &nCol, &nRowSpan, &nColSpan);

	m_pRichTextEdit = new QwwRichTextEdit(this);
	m_pRichTextEdit->setObjectName(QString::fromUtf8("textEdit"));
	m_pRichTextEdit->setMouseTracking(true);
//	m_pRichTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
//	m_pRichTextEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_pRichTextEdit->setTabChangesFocus(false);
	m_pRichTextEdit->setTextInteractionFlags(Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse|Qt::TextEditable);

	delete ui->textEdit;
	ui->textEdit = NULL;
	ui->gridLayoutMain->addWidget(m_pRichTextEdit, nRow, nCol, nRowSpan, nColSpan);

	// --------------------------------------------------------------

	//	Swapout the buttonBackgroundColor from the layout with a QwwColorButton:

//	ndx = ui->horizontalLayout->indexOf(ui->buttonBackgroundColor);
//	assert(ndx != -1);
//	if (ndx == -1) return;

	ndx = ui->gridLayoutControls->indexOf(ui->buttonBackgroundColor);
	assert(ndx != -1);
	if (ndx == -1) return;
	ui->gridLayoutControls->getItemPosition(ndx, &nRow, &nCol, &nRowSpan, &nColSpan);

	m_pBackgroundColorButton = new QwwColorButton(this);
	m_pBackgroundColorButton->setObjectName(QString::fromUtf8("buttonBackgroundColor"));
	m_pBackgroundColorButton->setShowName(false);			// Must do this before setting our real text
	m_pBackgroundColorButton->setText(tr("Note Background Color"));
	m_pBackgroundColorButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

	delete ui->buttonBackgroundColor;
	ui->buttonBackgroundColor = NULL;
//	ui->horizontalLayout->insertWidget(ndx, m_pBackgroundColorButton);
	ui->gridLayoutControls->addWidget(m_pBackgroundColorButton, nRow, nCol, nRowSpan, nColSpan);

	// --------------------------------------------------------------

	// Reinsert it in the correct TabOrder:
	QWidget::setTabOrder(m_pRichTextEdit, ui->buttonBox);
	QWidget::setTabOrder(ui->buttonBox, ui->editNoteLocation);
	QWidget::setTabOrder(ui->editNoteLocation, m_pBackgroundColorButton);
	QWidget::setTabOrder(m_pBackgroundColorButton, ui->widgetNoteKeywords);

	// --------------------------------------------------------------

	// Setup Dialog buttons:

	assert(ui->buttonBox->button(QDialogButtonBox::Ok) != NULL);
	ui->buttonBox->button(QDialogButtonBox::Ok)->setIcon(QIcon(":res/ok_blue-24.png"));
	assert(ui->buttonBox->button(QDialogButtonBox::Cancel) != NULL);
	ui->buttonBox->button(QDialogButtonBox::Cancel)->setIcon(QIcon(":res/cancel-24.png"));
	m_pDeleteNoteButton = ui->buttonBox->addButton(tr("Delete Note"), QDialogButtonBox::ActionRole);
	m_pDeleteNoteButton->setIcon(QIcon(":res/deletered1-24.png"));
	connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(en_ButtonClicked(QAbstractButton*)));

	// --------------------------------------------------------------

	connect(m_pRichTextEdit, SIGNAL(textChanged()), this, SLOT(en_textChanged()));
	connect(m_pBackgroundColorButton, SIGNAL(colorPicked(const QColor &)), this, SLOT(en_BackgroundColorPicked(const QColor &)));
	connect(ui->widgetNoteKeywords, SIGNAL(keywordListChanged()), this, SLOT(en_keywordListChanged()));

	m_pRichTextEdit->setFocus();
}

CKJVNoteEditDlg::~CKJVNoteEditDlg()
{
	delete ui;
}

// ============================================================================

void CKJVNoteEditDlg::writeSettings(QSettings &settings, const QString &prefix)
{
	// RestoreState:
	settings.beginGroup(groupCombine(prefix, constrRestoreStateGroup));
	settings.setValue(constrGeometryKey, saveGeometry());
	settings.endGroup();
}

void CKJVNoteEditDlg::readSettings(QSettings &settings, const QString &prefix)
{
	// RestoreState:
	settings.beginGroup(groupCombine(prefix, constrRestoreStateGroup));
	restoreGeometry(settings.value(constrGeometryKey).toByteArray());
	settings.endGroup();
}

// ============================================================================

void CKJVNoteEditDlg::setLocationIndex(const CRelIndex &ndxLocation)
{
	assert(m_pRichTextEdit != NULL);
	assert(g_pUserNotesDatabase != NULL);

	m_ndxLocation = ndxLocation;
	m_ndxLocation.setWord(0);		// Work with whole verses only
	ui->editNoteLocation->setText(m_pBibleDatabase->PassageReferenceText(m_ndxLocation));

	m_UserNote = g_pUserNotesDatabase->noteFor(m_ndxLocation);

	m_bDoingUpdate = true;

	m_pBackgroundColorButton->setCurrentColor(m_UserNote.backgroundColor());
	setBackgroundColorPreview();

	// Setup Keywords:
	ui->widgetNoteKeywords->setKeywordList(m_UserNote.keywordList(), g_pUserNotesDatabase->compositeKeywordList());

	m_pRichTextEdit->setHtml(m_UserNote.text());
	m_bIsDirty = false;

	m_bDoingUpdate = false;
}

// ============================================================================

void CKJVNoteEditDlg::accept()
{
	assert(m_pRichTextEdit != NULL);
	assert(g_pUserNotesDatabase != NULL);

	m_UserNote.setText(m_pRichTextEdit->toHtml());
	m_UserNote.setIsVisible(true);			// Make note visible when they are explicitly setting it
	g_pUserNotesDatabase->setNoteFor(m_ndxLocation, m_UserNote);
	m_bIsDirty = false;
	QDialog::accept();
}

void CKJVNoteEditDlg::reject()
{
	if (m_bIsDirty) {
		int nResult = QMessageBox::warning(this, windowTitle(), tr("You have made changes to this note.  Do you wish to discard them??"),
																	(QMessageBox::Ok | QMessageBox::Cancel), QMessageBox::Cancel);
		if (nResult != QMessageBox::Ok) return;
	}

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

// ============================================================================

void CKJVNoteEditDlg::en_ButtonClicked(QAbstractButton *button)
{
	assert(button != NULL);
	assert(g_pUserNotesDatabase != NULL);

	if (button == m_pDeleteNoteButton) {
		int nResult = QMessageBox::warning(this, windowTitle(), tr("Are you sure you want to completely delete this note??"),
																	(QMessageBox::Ok | QMessageBox::Cancel), QMessageBox::Cancel);
		if (nResult != QMessageBox::Ok) return;
		m_bIsDirty = false;
		g_pUserNotesDatabase->removeNoteFor(m_ndxLocation);
		QDialog::accept();
	}
}

// ============================================================================

void CKJVNoteEditDlg::en_keywordListChanged()
{
	if (m_bDoingUpdate) return;
	m_bDoingUpdate = true;

	m_UserNote.setKeywordList(ui->widgetNoteKeywords->selectedKeywordList());
	m_bIsDirty = true;

	m_bDoingUpdate = false;
}

// ============================================================================
