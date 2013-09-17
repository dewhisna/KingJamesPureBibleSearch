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

// ============================================================================

CDictionaryLineEdit::CDictionaryLineEdit(QWidget *pParent)
	:	CSingleLineTextEdit(-1, pParent),
		m_pCompleter(NULL)
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

	m_pCompleter->setCompletionFilterMode(CSearchCompleter::SCFME_SOUNDEX);		// CPersistentSettings::instance()->searchPhraseCompleterFilterMode());

	connect(this, SIGNAL(textChanged()), this, SLOT(en_textChanged()));

	connect(m_pCompleter, SIGNAL(activated(const QModelIndex &)), this, SLOT(insertCompletion(const QModelIndex &)));
//	connect(CPersistentSettings::instance(), SIGNAL(changedSearchPhraseCompleterFilterMode(CSearchCompleter::SEARCH_COMPLETION_FILTER_MODE_ENUM)), this, SLOT(en_changedSearchPhraseCompleterFilterMode(CSearchCompleter::SEARCH_COMPLETION_FILTER_MODE_ENUM)));


}

void CDictionaryLineEdit::setupCompleter(const QString &strText, bool bForce)
{

}


void CDictionaryLineEdit::en_textChanged()
{

}

void CDictionaryLineEdit::insertCompletion(const QModelIndex &index)
{

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
}

CDictionaryWidget::~CDictionaryWidget()
{

}

// ============================================================================
