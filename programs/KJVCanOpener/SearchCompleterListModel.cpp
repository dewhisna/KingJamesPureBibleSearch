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
#include "PhraseNavigator.h"

// ============================================================================

CSearchParsedPhraseListModel::CSearchParsedPhraseListModel(CParsedPhrase &parsedPhrase, QObject *parent)
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

	return basicWordsListSize();
}

QVariant CSearchParsedPhraseListModel::data(const QModelIndex &index, int role) const
{
	if ((index.row() < 0) || (index.row() >= basicWordsListSize()))
		return QVariant();

	if (role == Qt::DisplayRole)
		return (basicWordsListEntry(index.row()).searchWord());

	if (role == Qt::EditRole)
		return basicWordsListEntry(index.row()).decomposedWord();

	if (role == SOUNDEX_ENTRY_ROLE)
		return m_parsedPhrase.bibleDatabase()->soundEx(basicWordsListEntry(index.row()).decomposedWord());

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

		TWordListSet setPhraseWords;

		m_lstBasicWords.clear();
		if (!m_parsedPhrase.bibleDatabase()->searchSpaceIsCompleteConcordance()) {
			m_lstBasicWords.reserve(m_parsedPhrase.nextWordsList().size());
			for (int ndx = 0; ndx < m_parsedPhrase.nextWordsList().size(); ++ndx) {
				if (setPhraseWords.find(m_parsedPhrase.nextWordsList().at(ndx).searchWord()) == setPhraseWords.cend()) {
					setPhraseWords.insert(m_parsedPhrase.nextWordsList().at(ndx).searchWord());
					m_lstBasicWords.append(m_parsedPhrase.nextWordsList().at(ndx));
				}
			}
		}

		endResetModel();

		emit modelChanged();

	}
}

void CSearchParsedPhraseListModel::UpdateCompleter(const QTextCursor &curInsert)
{
	// This function would be used to update the list model
	//	for the text at the current cursor position:
	m_parsedPhrase.UpdateCompleter(curInsert);
}

int CSearchParsedPhraseListModel::basicWordsListSize() const
{
	if (m_lstBasicWords.size()) return m_lstBasicWords.size();
	return m_parsedPhrase.nextWordsList().size();
}

const CBasicWordEntry &CSearchParsedPhraseListModel::basicWordsListEntry(int ndx) const
{
	if (m_lstBasicWords.size()) return m_lstBasicWords.at(ndx);
	return m_parsedPhrase.nextWordsList().at(ndx);
}

// ============================================================================

CSearchDictionaryListModel::CSearchDictionaryListModel(CDictionaryDatabasePtr pDictionary, TEditorWordCallback pFuncEditorWord, QObject *parent)
	:	CSearchStringListModel(parent),
		m_pDictionaryDatabase(pDictionary),
		m_pFuncEditorWord(pFuncEditorWord)
{
	Q_ASSERT(!pDictionary.isNull());
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
		return m_pDictionaryDatabase->wordDefinitionsEntry(m_pDictionaryDatabase->wordEntry(index.row())).searchWord();

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
	return m_pFuncEditorWord();
}

void CSearchDictionaryListModel::setWordsFromPhrase(bool bForceUpdate)
{
	Q_UNUSED(bForceUpdate);

//	emit beginResetModel();
//	emit endResetModel();
//	emit modelChanged();
}

void CSearchDictionaryListModel::UpdateCompleter(const QTextCursor &curInsert)
{
	// This function would be used to update the list model
	//	for the text at the current cursor position.  This
	//	is currently a no-op for the dictionary model, since
	//	the dictionary model uses the entire text content
	//	and doesn't have to use something like CParsedPhrase
	//	that the CSearchParsedPhraseListModel does:
	Q_UNUSED(curInsert);
}

// ============================================================================

CSearchStrongsDictionaryListModel::CSearchStrongsDictionaryListModel(CDictionaryDatabasePtr pDictionary, TEditorWordCallback pFuncEditorWord, QObject *parent)
	:	QAbstractListModel(parent),
		m_pDictionaryDatabase(pDictionary),
		m_pFuncEditorWord(pFuncEditorWord)
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
	return m_pFuncEditorWord();
}

// ============================================================================

