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

#include "SubControls.h"

#include <QFontMetrics>
#include <QStyle>
#include <QApplication>
#include <QTextDocumentFragment>
#include <QAbstractItemView>

#include "Qt_QStyleOption_stub.h"

#if QT_VERSION < 0x050000
#include <QInputContext>
#endif

// ============================================================================

CComboBox::CComboBox(QWidget *pParent)
	:	QComboBox(pParent)
{

}

CComboBox::~CComboBox()
{

}

void CComboBox::keyPressEvent(QKeyEvent *pEvent)
{
	// let base class handle the event
	QComboBox::keyPressEvent(pEvent);

	if ((pEvent->key() == Qt::Key_Enter) || (pEvent->key() == Qt::Key_Return)) {
		// Process enter/return so it won't propagate and "accept" the parent dialog:
		pEvent->accept();
		emit enterPressed();
	}
}

// ============================================================================

CFontComboBox::CFontComboBox(QWidget *pParent)
	:	QFontComboBox(pParent)
{

}

CFontComboBox::~CFontComboBox()
{

}

void CFontComboBox::keyPressEvent(QKeyEvent *pEvent)
{
	// let base class handle the event
	QFontComboBox::keyPressEvent(pEvent);

	if ((pEvent->key() == Qt::Key_Enter) || (pEvent->key() == Qt::Key_Return)) {
		// Process enter/return so it won't propagate and "accept" the parent dialog:
		pEvent->accept();
		emit enterPressed();
	}
}

// ============================================================================

CSpinBox::CSpinBox(QWidget *pParent)
	:	QSpinBox(pParent)
{

}

CSpinBox::~CSpinBox()
{

}

void CSpinBox::keyPressEvent(QKeyEvent *pEvent)
{
	// let base class handle the event
	QSpinBox::keyPressEvent(pEvent);

	if ((pEvent->key() == Qt::Key_Enter) || (pEvent->key() == Qt::Key_Return)) {
		// Process enter/return so it won't propagate and "accept" the parent dialog:
		pEvent->accept();
		emit enterPressed();
	}
}

// ============================================================================

CDoubleSpinBox::CDoubleSpinBox(QWidget *pParent)
	:	QDoubleSpinBox(pParent)
{

}

CDoubleSpinBox::~CDoubleSpinBox()
{

}

void CDoubleSpinBox::keyPressEvent(QKeyEvent *pEvent)
{
	// let base class handle the event
	QDoubleSpinBox::keyPressEvent(pEvent);

	if ((pEvent->key() == Qt::Key_Enter) || (pEvent->key() == Qt::Key_Return)) {
		// Process enter/return so it won't propagate and "accept" the parent dialog:
		pEvent->accept();
		emit enterPressed();
	}
}

// ============================================================================

CSingleLineTextEdit::CSingleLineTextEdit(int nMinHeight, QWidget *pParent)
	:	QTextEdit(pParent),
		m_nMinHeight(nMinHeight),
		m_bUpdateInProgress(false)
{
	connect(this, SIGNAL(textChanged()), this, SLOT(en_textChanged()));
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(en_cursorPositionChanged()));
}

CSingleLineTextEdit::~CSingleLineTextEdit()
{

}

QSize CSingleLineTextEdit::sizeHint() const
{
	QFontMetrics fm(font());
	int h = qMax(fm.height(), 14) + 4;
#if QT_VERSION >= 0x050B00
	int w = fm.horizontalAdvance(QLatin1Char('x')) * 17 + 4;
#else
	int w = fm.width(QLatin1Char('x')) * 17 + 4;
#endif
	QStyleOptionFrameV2_t opt;
	opt.initFrom(this);
#if QT_VERSION >= 0x060000
	QSize szHint = style()->sizeFromContents(QStyle::CT_LineEdit, &opt, QSize(w, h), this);
#else
	QSize szHint = style()->sizeFromContents(QStyle::CT_LineEdit, &opt, QSize(w, h).
											 expandedTo(QApplication::globalStrut()), this);
#endif
	return (QSize(szHint.width(), ((m_nMinHeight != -1) ? qMax(szHint.height(), m_nMinHeight) : szHint.height())));
}

void CSingleLineTextEdit::insertFromMimeData(const QMimeData * source)
{
	if (!(textInteractionFlags() & Qt::TextEditable) || !source) return;

	// For reference if we ever re-enable rich text:  (don't forget to change acceptRichText setting in constructor)
	//	if (source->hasFormat(QLatin1String("application/x-qrichtext")) && acceptRichText()) {
	//		// x-qrichtext is always UTF-8 (taken from Qt3 since we don't use it anymore).
	//		QString richtext = QString::fromUtf8(source->data(QLatin1String("application/x-qrichtext")));
	//		richtext.prepend(QLatin1String("<meta name=\"qrichtext\" content=\"1\" />"));
	//		fragment = QTextDocumentFragment::fromHtml(richtext, document());
	//		bHasData = true;
	//	} else if (source->hasHtml() && acceptRichText()) {
	//		fragment = QTextDocumentFragment::fromHtml(source->html(), document());
	//		bHasData = true;
	//	} else {


	if (source->hasText()) {
		bool bHasData = false;
		QTextDocumentFragment fragment;
		QString text = source->text();
		// Change all newlines to spaces, since we are simulating a single-line editor:
		if (!text.isNull()) {
			text.replace("\r","");
			text.replace("\n"," ");
			if (!text.isEmpty()) {
				fragment = QTextDocumentFragment::fromPlainText(text);
				bHasData = true;
			}
		}
		if (bHasData) textCursor().insertFragment(fragment);
	}

	ensureCursorVisible();
}

bool CSingleLineTextEdit::canInsertFromMimeData(const QMimeData *source) const
{
	return QTextEdit::canInsertFromMimeData(source);
}

void CSingleLineTextEdit::wheelEvent(QWheelEvent *event)
{
	event->ignore();
}

void CSingleLineTextEdit::focusInEvent(QFocusEvent *event)
{
	QTextEdit::focusInEvent(event);

#if QT_VERSION < 0x050000
	// The following is needed to fix the QCompleter bug
	//	where the inputContext doesn't shift correctly
	//	from the QCompleter->popup back to the editor:
	//	(Only applies to Qt 4.8.x and seems to be fixed in Qt5)
	QInputContext *pInputContext = inputContext();
	if (pInputContext) pInputContext->setFocusWidget(this);
#endif
}

void CSingleLineTextEdit::keyPressEvent(QKeyEvent *event)
{
	bool bForceCompleter = false;

	switch (event->key()) {
		case Qt::Key_Enter:
		case Qt::Key_Return:
			emit enterTriggered();
			// fall-through to the event-ignore() so we can still process it for the completer logic
#ifdef __GNUC__
			[[gnu::fallthrough]];
#endif
		case Qt::Key_Escape:
		case Qt::Key_Tab:
		case Qt::Key_Control:			// Control is needed here to keep Ctrl-Home/Ctrl-End used in the QCompleter from trigger redoing the QCompleter in setupCompleter()
			event->ignore();
			return;

		case Qt::Key_Backspace:
			if (textInteractionFlags() & Qt::TextEditable) {
				// Normally, the backspace key only deletes the next QChar,
				//	which for marks is just the mark on the character, unlike
				//	the delete key which deletes the next character along with
				//	its marks.  So, if there are any marks delete them and
				//	then fallthrough to call the original control behavior to
				//	delete the character itself:
				QTextDocument *pDocument = document();
				QTextCursor cursor = textCursor();
				if (cursor.anchor() == cursor.position()) {		// If we have no selection, delete marks for characters too:
					if (pDocument) {
						cursor.beginEditBlock();
						while ((cursor.position() > 1) && pDocument->characterAt(cursor.position()-1).isMark()) {
							cursor.deletePreviousChar();
						}
						cursor.endEditBlock();
					}
				}
			}
			break;

		case Qt::Key_Down:
			bForceCompleter = true;
			break;
	}

	QTextEdit::keyPressEvent(event);

	// Ctrl/Cmd modifier is needed here to keep QCompleter from triggering on our editor shortcuts via setupCompleter()
	if (event->modifiers() & Qt::ControlModifier) return;

	setupCompleter(event->text(), bForceCompleter);
}

void CSingleLineTextEdit::inputMethodEvent(QInputMethodEvent *event)
{
	// Call parent:
	QTextEdit::inputMethodEvent(event);
	setupCompleter(event->commitString(), false);
}

QString CSingleLineTextEdit::textUnderCursor() const
{
	QTextCursor cursor = textCursor();
	cursor.select(QTextCursor::WordUnderCursor);
	return cursor.selectedText();
}

void CSingleLineTextEdit::en_textChanged()
{
	if (!updateInProgress()) UpdateCompleter();
}

void CSingleLineTextEdit::en_cursorPositionChanged()
{
	if (!updateInProgress()) UpdateCompleter();
}

// ============================================================================
