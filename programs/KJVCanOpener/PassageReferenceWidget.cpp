/****************************************************************************
**
** Copyright (C) 2013-2022 Donna Whisnant, a.k.a. Dewtronics.
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

#include "PassageReferenceWidget.h"

#include "PassageReferenceResolver.h"

#include <QColor>
#include <QKeyEvent>
#include <QMenu>

// ============================================================================

CPassageReferenceWidget::CPassageReferenceWidget(QWidget *parent)
	:	QWidget(parent),
		m_pEditMenu(nullptr),
		m_pActionUndo(nullptr),
		m_pActionRedo(nullptr),
		m_pActionCut(nullptr),
		m_pActionCopy(nullptr),
		m_pActionPaste(nullptr),
		m_pActionDelete(nullptr),
		m_pActionSelectAll(nullptr)
{
	ui.setupUi(this);

	// ------------------------------------------------------------------------

	m_pEditMenu = new QMenu(tr("&Edit", "MainMenu"), ui.editPassageReference);
	m_pEditMenu->setStatusTip(tr("Passage Reference Editor Operations", "MainMenu"));

	m_pActionUndo = m_pEditMenu->addAction(tr("&Undo", "MainMenu"), ui.editPassageReference, SLOT(undo()), QKeySequence(Qt::CTRL | Qt::Key_Z));
	m_pActionUndo->setStatusTip(tr("Undo last operation to the Passage Reference Editor", "MainMenu"));
	m_pActionUndo->setEnabled(false);
//	connect(m_pActionUndo, SIGNAL(triggered()), ui.editPassageReference, SLOT(setFocus()));
	m_pActionRedo = m_pEditMenu->addAction(tr("&Redo", "MainMenu"), ui.editPassageReference, SLOT(redo()), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Z));
	m_pActionRedo->setStatusTip(tr("Redo last operation on the Passage Reference Editor", "MainMenu"));
	m_pActionRedo->setEnabled(false);
//	connect(m_pActionRedo, SIGNAL(triggered()), ui.editPassageReference, SLOT(setFocus()));
	m_pEditMenu->addSeparator();
	m_pActionCut = m_pEditMenu->addAction(tr("Cu&t", "MainMenu"), ui.editPassageReference, SLOT(cut()), QKeySequence(Qt::CTRL | Qt::Key_X));
	m_pActionCut->setStatusTip(tr("Cut selected text from the Passage Reference Editor to the clipboard", "MainMenu"));
	m_pActionCut->setEnabled(false);
//	connect(m_pActionCut, SIGNAL(triggered()), ui.editPassageReference, SLOT(setFocus()));
	m_pActionCopy = m_pEditMenu->addAction(tr("&Copy", "MainMenu"), ui.editPassageReference, SLOT(copy()), QKeySequence(Qt::CTRL | Qt::Key_C));
	m_pActionCopy->setStatusTip(tr("Copy selected text from the Passage Reference Editor to the clipboard", "MainMenu"));
	m_pActionCopy->setEnabled(false);
//	connect(m_pActionCopy, SIGNAL(triggered()), ui.editPassageReference, SLOT(setFocus()));
	m_pActionPaste = m_pEditMenu->addAction(tr("&Paste", "MainMenu"), ui.editPassageReference, SLOT(paste()), QKeySequence(Qt::CTRL | Qt::Key_V));
	m_pActionPaste->setStatusTip(tr("Paste text on clipboard into the Passage Reference Editor", "MainMenu"));
	m_pActionPaste->setEnabled(true);
//	connect(m_pActionPaste, SIGNAL(triggered()), ui.editPassageReference, SLOT(setFocus()));
	m_pActionDelete = m_pEditMenu->addAction(tr("&Delete", "MainMenu"), ui.editPassageReference, SLOT(clear()), QKeySequence(Qt::Key_Delete));
	m_pActionDelete->setStatusTip(tr("Delete selected text from the Passage Reference Editor", "MainMenu"));
	m_pActionDelete->setEnabled(false);
//	connect(m_pActionDelete, SIGNAL(triggered()), ui.editPassageReference, SLOT(setFocus()));
	m_pEditMenu->addSeparator();
	m_pActionSelectAll = m_pEditMenu->addAction(tr("Select &All", "MainMenu"), ui.editPassageReference, SLOT(selectAll()), QKeySequence(Qt::CTRL | Qt::Key_A));
	m_pActionSelectAll->setStatusTip(tr("Select All Text in the Passage Reference Editor", "MainMenu"));
	m_pActionSelectAll->setEnabled(false);
//	connect(m_pActionSelectAll, SIGNAL(triggered()), ui.editPassageReference, SLOT(setFocus()));

	connect(ui.editPassageReference, SIGNAL(textChanged(QString)), this, SLOT(en_setMenuEnables(QString)));

	ui.editPassageReference->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui.editPassageReference, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(en_passageReferenceContextMenuRequested(QPoint)));

	ui.editPassageReference->installEventFilter(this);

	// ------------------------------------------------------------------------

	connect(ui.editPassageReference, SIGNAL(textChanged(QString)), this, SLOT(en_PassageReferenceChanged(QString)));
}

CPassageReferenceWidget::~CPassageReferenceWidget()
{

}

void CPassageReferenceWidget::initialize(CBibleDatabasePtr pBibleDatabase)
{
	if (!m_pRefResolver.isNull()) delete m_pRefResolver.data();
	m_pRefResolver = new CPassageReferenceResolver(pBibleDatabase, this);
}

bool CPassageReferenceWidget::hasFocusPassageReferenceEditor() const
{
	return ui.editPassageReference->hasFocus();
}

void CPassageReferenceWidget::clear()
{
	m_tagPhrase = TPhraseTag();
	ui.editPassageReference->clear();
}

void CPassageReferenceWidget::setPassageReference(const QString &strPassageReference)
{
	ui.editPassageReference->setText(strPassageReference);
}

bool CPassageReferenceWidget::eventFilter(QObject *pObject, QEvent *pEvent)
{
	Q_ASSERT(pEvent != nullptr);
	if ((pObject == ui.editPassageReference) && (pEvent->type() == QEvent::FocusIn)) emit activatedPassageReference();

	return QWidget::eventFilter(pObject, pEvent);
}

void CPassageReferenceWidget::focusInEvent(QFocusEvent *event)
{
	QWidget::focusInEvent(event);
	ui.editPassageReference->setFocus();
}

void CPassageReferenceWidget::keyPressEvent(QKeyEvent *event)
{
	if ((event) &&
		((event->key() == Qt::Key_Enter) ||
		 (event->key() == Qt::Key_Return))) {
		// Process enter/return so it won't propagate and "accept" the parent dialog:
		event->accept();
		emit enterPressed();
	} else {
		QWidget::keyPressEvent(event);
	}
}

void CPassageReferenceWidget::en_passageReferenceContextMenuRequested(const QPoint &pos)
{
	Q_ASSERT(m_pEditMenu != nullptr);
#ifndef USE_ASYNC_DIALOGS
	m_pEditMenu->exec(ui.editPassageReference->mapToGlobal(pos));
#else
	m_pEditMenu->popup(ui.editPassageReference->mapToGlobal(pos));
#endif
}

void CPassageReferenceWidget::en_setMenuEnables(const QString &strText)
{
	m_pActionUndo->setEnabled(ui.editPassageReference->isUndoAvailable());
	m_pActionRedo->setEnabled(ui.editPassageReference->isRedoAvailable());
	m_pActionCut->setEnabled(ui.editPassageReference->hasSelectedText());
	m_pActionCopy->setEnabled(ui.editPassageReference->hasSelectedText());
	m_pActionDelete->setEnabled(!strText.isEmpty());
	m_pActionSelectAll->setEnabled(!strText.isEmpty());
}

void CPassageReferenceWidget::en_PassageReferenceChanged(const QString &strText)
{
	Q_ASSERT(!m_pRefResolver.isNull());		// Run initialize first!
	if (m_pRefResolver.isNull()) return;

	m_tagPhrase = m_pRefResolver->resolve(strText);
	if ((m_tagPhrase.relIndex().isSet()) || (strText.trimmed().isEmpty())) {
		ui.editPassageReference->setStyleSheet(QString());
	} else {
		ui.editPassageReference->setStyleSheet(QString("QLineEdit { color:%1; }").arg(QColor("red").name()));
	}

	emit passageReferenceChanged(m_tagPhrase);
}

// ============================================================================

