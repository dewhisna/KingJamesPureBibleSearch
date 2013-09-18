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

#include "DictionaryWidget.h"

#include "PersistentSettings.h"

#include <QTextCursor>
#include <QTextCharFormat>

// ============================================================================

CDictionaryLineEdit::CDictionaryLineEdit(QWidget *pParent)
	:	CSingleLineTextEdit(-1, pParent),
		m_pCompleter(NULL),
		m_bUpdateInProgress(false)
{

}

CDictionaryLineEdit::~CDictionaryLineEdit()
{

}

void CDictionaryLineEdit::initialize(CDictionaryDatabasePtr pDictionary)
{
	assert(pDictionary != NULL);
	m_pDictionaryDatabase = pDictionary;

	setAcceptRichText(false);
	setUndoRedoEnabled(false);		// TODO : If we ever address what to do with undo/redo, then re-enable this

	setTabChangesFocus(true);
	setWordWrapMode(QTextOption::NoWrap);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	setFixedHeight(sizeHint().height());
	setLineWrapMode(QTextEdit::NoWrap);

	m_pCompleter = new SearchCompleter_t(m_pDictionaryDatabase, *this, this);
//	m_pCompleter->setCaseSensitivity(isCaseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive);
	// TODO : ??? Add AccentSensitivity to completer ???

	m_pCompleter->setCompletionFilterMode(CSearchCompleter::SCFME_NORMAL);		// CPersistentSettings::instance()->searchPhraseCompleterFilterMode());

	connect(m_pCompleter, SIGNAL(activated(const QModelIndex &)), this, SLOT(insertCompletion(const QModelIndex &)));
//	connect(CPersistentSettings::instance(), SIGNAL(changedSearchPhraseCompleterFilterMode(CSearchCompleter::SEARCH_COMPLETION_FILTER_MODE_ENUM)), this, SLOT(en_changedSearchPhraseCompleterFilterMode(CSearchCompleter::SEARCH_COMPLETION_FILTER_MODE_ENUM)));
}

void CDictionaryLineEdit::en_cursorPositionChanged()
{
	// Not calling base class because we don't need to update the completer on the
	//		cursor move (unlike the Search Phrases do)
}

void CDictionaryLineEdit::setupCompleter(const QString &strText, bool bForce)
{
	QString strWord = toPlainText();

	bool bCompleterOpen = m_pCompleter->popup()->isVisible();
	if ((bForce) || (!strText.isEmpty()) || (bCompleterOpen)) {
		m_pCompleter->setFilterMatchString();
		UpdateCompleter();
		m_pCompleter->popup()->close();
		if ((bCompleterOpen) && (strWord.length() > 2)) bForce = true;				// Reshow completer if it was open already and we're changing words
		m_pCompleter->selectFirstMatchString();
	}

	if (bForce || (!strText.isEmpty() && (strWord.length() > 2)))
		m_pCompleter->complete();
}

void CDictionaryLineEdit::UpdateCompleter()
{
	m_pCompleter->setWordsFromPhrase();

	if (updateInProgress()) return;
	CDoUpdate doUpdate(this);

	QTextCursor saveCursor = textCursor();

	QTextCursor cursor(textCursor());
	QTextCharFormat fmt = cursor.charFormat();
	fmt.setFontStrikeOut(false);
	fmt.setUnderlineStyle(QTextCharFormat::NoUnderline);

	cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
	cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
	cursor.setCharFormat(fmt);

	if (!m_pDictionaryDatabase->wordExists(toPlainText().trimmed())) {
		fmt.setFontStrikeOut(true);
		fmt.setUnderlineColor(QColor(255,0,0));
		fmt.setUnderlineStyle(QTextCharFormat::WaveUnderline);
		cursor.setCharFormat(fmt);
	}

	setTextCursor(saveCursor);
}

void CDictionaryLineEdit::insertCompletion(const QModelIndex &index)
{
	insertCompletion(m_pCompleter->completionModel()->data(index, Qt::DisplayRole).toString());
}

void CDictionaryLineEdit::insertCompletion(const QString &strWord)
{
	QTextCursor cursor(textCursor());
	cursor.beginEditBlock();
	cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
	cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
	cursor.insertText(strWord);
	cursor.endEditBlock();
}

// ============================================================================

CDictionaryWidget::CDictionaryWidget(CDictionaryDatabasePtr pDictionary, QWidget *parent)
	:	QWidget(parent),
		m_pDictionaryDatabase(pDictionary),
		m_bDoingPopup(false)
{
	assert(m_pDictionaryDatabase != NULL);

	ui.setupUi(this);

	ui.editDictionaryWord->initialize(m_pDictionaryDatabase);

	connect(ui.editDictionaryWord, SIGNAL(textChanged()), this, SLOT(en_wordChanged()));

	setTextBrightness(CPersistentSettings::instance()->invertTextBrightness(), CPersistentSettings::instance()->textBrightness());
	connect(CPersistentSettings::instance(), SIGNAL(changedTextBrightness(bool, int)), this, SLOT(setTextBrightness(bool, int)));
}

CDictionaryWidget::~CDictionaryWidget()
{

}

void CDictionaryWidget::setWord(const QString &strWord)
{
	ui.editDictionaryWord->insertCompletion(strWord);
}

void CDictionaryWidget::en_wordChanged()
{
	ui.definitionBrowser->setHtml(m_pDictionaryDatabase->definition(ui.editDictionaryWord->toPlainText().trimmed()));
}

void CDictionaryWidget::setTextBrightness(bool bInvert, int nBrightness)
{
	setStyleSheet(QString("CDictionaryLineEdit, QTextBrowser { background-color:%1; color:%2; }")
								   .arg(CPersistentSettings::textBackgroundColor(bInvert, nBrightness).name())
								   .arg(CPersistentSettings::textForegroundColor(bInvert, nBrightness).name()));
}

// ============================================================================
