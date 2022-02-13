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

#include "SearchCompleterListModel.h"
#include "PhraseEdit.h"

#ifdef QT_WIDGETS_LIB
#include <QTextEdit>
#endif

// ============================================================================

CSearchParsedPhraseListModel::CSearchParsedPhraseListModel(const CParsedPhrase &parsedPhrase, QObject *parent)
	:	CSearchStringListModel(parent),
		m_parsedPhrase(parsedPhrase),
		m_nCursorWord(-1)			// Force initial update
{

}

CSearchParsedPhraseListModel::~CSearchParsedPhraseListModel()
{

}

int CSearchParsedPhraseListModel::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return 0;

	return m_parsedPhrase.nextWordsList().size();
}

QVariant CSearchParsedPhraseListModel::data(const QModelIndex &index, int role) const
{
	if ((index.row() < 0) || (index.row() >= m_parsedPhrase.nextWordsList().size()))
		return QVariant();

	if (role == Qt::DisplayRole)
		return (m_parsedPhrase.nextWordsList().at(index.row()).renderedWord());

	if (role == Qt::EditRole)
		return m_parsedPhrase.nextWordsList().at(index.row()).decomposedWord();

	if (role == SOUNDEX_ENTRY_ROLE)
		return m_parsedPhrase.bibleDatabase()->soundEx(m_parsedPhrase.nextWordsList().at(index.row()).decomposedWord());

	return QVariant();
}

bool CSearchParsedPhraseListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	Q_UNUSED(index);
	Q_UNUSED(value);
	Q_UNUSED(role);

	return false;
}

QString CSearchParsedPhraseListModel::soundEx(const QString &strDecomposedWord, bool bCache) const
{
	return m_parsedPhrase.bibleDatabase()->soundEx(strDecomposedWord, bCache);
}

QString CSearchParsedPhraseListModel::cursorWord() const
{
	return m_parsedPhrase.GetCursorWord();
}

void CSearchParsedPhraseListModel::setWordsFromPhrase(bool bForceUpdate)
{
#ifdef SEARCH_COMPLETER_DEBUG_OUTPUT
	qDebug("SearchStringListModel::setWordsFromPhrase : %d  OldCursorPos: %d", m_parsedPhrase.GetCursorWordPos(), m_nCursorWord);
#endif

	if ((m_parsedPhrase.GetCursorWordPos() != m_nCursorWord) || (bForceUpdate)) {
		m_nCursorWord = m_parsedPhrase.GetCursorWordPos();

		beginResetModel();

//		m_ParsedPhrase.nextWordsList();

		m_lstBasicWords.clear();
		m_lstBasicWords.reserve(m_parsedPhrase.nextWordsList().size());
		for (int ndx = 0; ndx < m_parsedPhrase.nextWordsList().size(); ++ndx) {
			m_lstBasicWords.append(&m_parsedPhrase.nextWordsList().at(ndx));
		}

		endResetModel();

		emit modelChanged();

		// Free our list since it's only valid immediately after this function runs anyway:
		m_lstBasicWords.clear();

	}
}

// ============================================================================

#ifdef QT_WIDGETS_LIB

CSearchDictionaryListModel::CSearchDictionaryListModel(CDictionaryDatabasePtr pDictionary, const QTextEdit &editorWord, QObject *parent)
	:	CSearchStringListModel(parent),
		m_pDictionaryDatabase(pDictionary),
		m_editorWord(editorWord)
{
	Q_ASSERT(!pDictionary.isNull());

	m_lstBasicWords.clear();
	m_lstBasicWords.reserve(m_pDictionaryDatabase->wordCount());
	for (int ndx = 0; ndx < m_pDictionaryDatabase->wordCount(); ++ndx) {
		m_lstBasicWords.append(&m_pDictionaryDatabase->wordDefinitionsEntry(m_pDictionaryDatabase->wordEntry(ndx)));
	}
}

CSearchDictionaryListModel::~CSearchDictionaryListModel()
{

}

int CSearchDictionaryListModel::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return 0;

	return m_pDictionaryDatabase->wordCount();
}

QVariant CSearchDictionaryListModel::data(const QModelIndex &index, int role) const
{
	if ((index.row() < 0) || (index.row() >= m_pDictionaryDatabase->wordCount()))
		return QVariant();

	if (role == Qt::DisplayRole)
		return m_pDictionaryDatabase->wordDefinitionsEntry(m_pDictionaryDatabase->wordEntry(index.row())).renderedWord();

	if (role == Qt::EditRole)
		return m_pDictionaryDatabase->wordDefinitionsEntry(m_pDictionaryDatabase->wordEntry(index.row())).decomposedWord();

	if (role == SOUNDEX_ENTRY_ROLE)
		return m_pDictionaryDatabase->soundEx(m_pDictionaryDatabase->wordEntry(index.row()));

	return QVariant();
}

bool CSearchDictionaryListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	Q_UNUSED(index);
	Q_UNUSED(value);
	Q_UNUSED(role);

	return false;
}

QString CSearchDictionaryListModel::soundEx(const QString &strDecomposedWord, bool bCache) const
{
	return m_pDictionaryDatabase->soundEx(strDecomposedWord, bCache);
}

QString CSearchDictionaryListModel::cursorWord() const
{
	return m_editorWord.toPlainText();
}

void CSearchDictionaryListModel::setWordsFromPhrase(bool bForceUpdate)
{
	Q_UNUSED(bForceUpdate)

//	emit beginResetModel();
//	emit endResetModel();
//	emit modelChanged();
}

#endif

// ============================================================================

#ifdef QT_WIDGETS_LIB

CSearchStrongsDictionaryListModel::CSearchStrongsDictionaryListModel(CDictionaryDatabasePtr pDictionary, const QTextEdit &editorWord, QObject *parent)
	:	QAbstractListModel(parent),
		m_pDictionaryDatabase(pDictionary),
		m_editorWord(editorWord)
{
	Q_ASSERT(!pDictionary.isNull());
}

CSearchStrongsDictionaryListModel::~CSearchStrongsDictionaryListModel()
{
}

int CSearchStrongsDictionaryListModel::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return 0;

	return m_pDictionaryDatabase->wordCount();
}

QVariant CSearchStrongsDictionaryListModel::data(const QModelIndex &index, int role) const
{
	if ((index.row() < 0) || (index.row() >= m_pDictionaryDatabase->wordCount()))
		return QVariant();

	if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
		return m_pDictionaryDatabase->wordEntry(index.row());

	return QVariant();
}

QString CSearchStrongsDictionaryListModel::cursorWord() const
{
	return m_editorWord.toPlainText();
}

#endif

// ============================================================================

