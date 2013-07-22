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
#include "UserNotesDatabase.h"

#include <QGridLayout>
#include <QByteArray>
#include <QMessageBox>

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
	:	QDialog(parent),
		ui(new Ui::CKJVNoteEditDlg),
		m_pBibleDatabase(pBibleDatabase),
		m_bIsDirty(false),
		m_pRichTextEdit(NULL)
{
	assert(pBibleDatabase != NULL);

	ui->setupUi(this);

	// --------------------------------------------------------------

	//	Swapout the textEdit from the layout with a QwwRichTextEdit:

	int ndx = ui->gridLayout->indexOf(ui->textEdit);
	assert(ndx != -1);
	if (ndx == -1) return;
	int nRow;
	int nCol;
	int nRowSpan;
	int nColSpan;
	ui->gridLayout->getItemPosition(ndx, &nRow, &nCol, &nRowSpan, &nColSpan);

	m_pRichTextEdit = new QwwRichTextEdit(this);
	m_pRichTextEdit->setObjectName(QString::fromUtf8("textEdit"));
	m_pRichTextEdit->setMouseTracking(true);
//	m_pRichTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
//	m_pRichTextEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_pRichTextEdit->setTabChangesFocus(false);
	m_pRichTextEdit->setTextInteractionFlags(Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse|Qt::TextEditable);

	delete ui->textEdit;
	ui->textEdit = NULL;
	ui->gridLayout->addWidget(m_pRichTextEdit, nRow, nCol, nRowSpan, nColSpan);

	// Reinsert it in the correct TabOrder:
	QWidget::setTabOrder(m_pRichTextEdit, ui->buttonBox);
	QWidget::setTabOrder(ui->buttonBox, ui->editNoteLocation);

	// --------------------------------------------------------------

	connect(m_pRichTextEdit, SIGNAL(textChanged()), this, SLOT(en_textChanged()));
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

	m_pRichTextEdit->setHtml(g_pUserNotesDatabase->noteFor(m_ndxLocation).text());
	m_bIsDirty = false;
}

// ============================================================================

void CKJVNoteEditDlg::accept()
{
	assert(m_pRichTextEdit != NULL);
	assert(g_pUserNotesDatabase != NULL);

	CUserNoteEntry userNote = g_pUserNotesDatabase->noteFor(m_ndxLocation);
	userNote.setText(m_pRichTextEdit->toHtml());
	g_pUserNotesDatabase->setNoteFor(m_ndxLocation, userNote);
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

void CKJVNoteEditDlg::en_textChanged()
{
	m_bIsDirty = true;
}

