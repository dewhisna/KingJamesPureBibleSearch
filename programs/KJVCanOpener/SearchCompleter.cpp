/****************************************************************************
**
** Copyright (C) 2013-2020 Donna Whisnant, a.k.a. Dewtronics.
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

#include "SearchCompleter.h"
#include "PhraseEdit.h"
#include "ParseSymbols.h"
#include "BusyCursor.h"

#include <QString>
#if QT_VERSION >= 0x050000
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#endif
#if QT_VERSION < 0x050F00
#include <QRegExp>
#endif
#include <QTimer>

#ifdef QT_WIDGETS_LIB
#include <QAbstractItemView>
#endif

#include <algorithm>

#if QT_VERSION < 0x050000
#include <QInputContext>
#endif

// ============================================================================

class TBasicWordHelper
{
public:
	TBasicWordHelper(const TBasicWordList &aWordList)
		:	m_lstBasicWords(aWordList)
	{

	}

	int indexOf_renderedWord(const QString &strWord, int nFrom = 0) const
	{
		if (nFrom < 0)
			nFrom = qMax(nFrom + m_lstBasicWords.size(), 0);
		if (nFrom < m_lstBasicWords.size()) {
			for (int ndx = nFrom; ndx < m_lstBasicWords.size(); ++ndx) {
				if (m_lstBasicWords.at(ndx)->renderedWord() == strWord) return ndx;
			}
		}
		return -1;
	}

	int lastIndexOf_renderedWord(const QString &strWord, int nFrom = -1) const
	{
		if (nFrom < 0)
			nFrom += m_lstBasicWords.size();
		else if (nFrom >= m_lstBasicWords.size())
			nFrom = m_lstBasicWords.size()-1;
		if (nFrom >= 0) {
			for (int ndx = nFrom; ndx >= 0; --ndx) {
				if (m_lstBasicWords.at(ndx)->renderedWord() == strWord) return ndx;
			}
		}
		return -1;
	}

	int indexOf_decomposedWord(const QString &strWord, int nFrom = 0) const
	{
		if (nFrom < 0)
			nFrom = qMax(nFrom + m_lstBasicWords.size(), 0);
		if (nFrom < m_lstBasicWords.size()) {
			for (int ndx = nFrom; ndx < m_lstBasicWords.size(); ++ndx) {
				if (m_lstBasicWords.at(ndx)->decomposedWord() == strWord) return ndx;
			}
		}
		return -1;
	}

	int lastIndexOf_decomposedWord(const QString &strWord, int nFrom = -1) const
	{
		if (nFrom < 0)
			nFrom += m_lstBasicWords.size();
		else if (nFrom >= m_lstBasicWords.size())
			nFrom = m_lstBasicWords.size()-1;
		if (nFrom >= 0) {
			for (int ndx = nFrom; ndx >= 0; --ndx) {
				if (m_lstBasicWords.at(ndx)->decomposedWord() == strWord) return ndx;
			}
		}
		return -1;
	}

#if QT_VERSION >= 0x050F00
	int indexOf_renderedWord(const QRegularExpression &rx, int nFrom = 0) const
#else
	int indexOf_renderedWord(const QRegExp &rx, int nFrom = 0) const
#endif
	{
		if (nFrom < 0)
			nFrom = qMax(nFrom + m_lstBasicWords.size(), 0);
		for (int i = nFrom; i < m_lstBasicWords.size(); ++i) {
#if QT_VERSION >= 0x050F00
			if (rx.match(m_lstBasicWords.at(i)->renderedWord()).hasMatch())
#else
			if (rx.exactMatch(m_lstBasicWords.at(i)->renderedWord()))
#endif
				return i;
		}
		return -1;
	}

#if QT_VERSION >= 0x050F00
	int lastIndexOf_renderedWord(const QRegularExpression &rx, int nFrom = -1) const
#else
	int lastIndexOf_renderedWord(const QRegExp &rx, int nFrom = -1) const
#endif
	{
		if (nFrom < 0)
			nFrom += m_lstBasicWords.size();
		else if (nFrom >= m_lstBasicWords.size())
			nFrom = m_lstBasicWords.size() - 1;
		for (int i = nFrom; i >= 0; --i) {
#if QT_VERSION >= 0x050F00
			if (rx.match(m_lstBasicWords.at(i)->renderedWord()).hasMatch())
#else
			if (rx.exactMatch(m_lstBasicWords.at(i)->renderedWord()))
#endif
				return i;
			}
		return -1;
	}

#if QT_VERSION >= 0x050F00
	int indexOf_decomposedWord(const QRegularExpression &rx, int nFrom = 0) const
#else
	int indexOf_decomposedWord(const QRegExp &rx, int nFrom = 0) const
#endif
	{
		if (nFrom < 0)
			nFrom = qMax(nFrom + m_lstBasicWords.size(), 0);
		for (int i = nFrom; i < m_lstBasicWords.size(); ++i) {
#if QT_VERSION >= 0x050F00
			if (rx.match(m_lstBasicWords.at(i)->decomposedWord()).hasMatch())
#else
			if (rx.exactMatch(m_lstBasicWords.at(i)->decomposedWord()))
#endif
				return i;
		}
		return -1;
	}

#if QT_VERSION >= 0x050F00
	int lastIndexOf_decomposedWord(const QRegularExpression &rx, int nFrom = -1) const
#else
	int lastIndexOf_decomposedWord(const QRegExp &rx, int nFrom = -1) const
#endif
	{
		if (nFrom < 0)
			nFrom += m_lstBasicWords.size();
		else if (nFrom >= m_lstBasicWords.size())
			nFrom = m_lstBasicWords.size() - 1;
		for (int i = nFrom; i >= 0; --i) {
#if QT_VERSION >= 0x050F00
			if (rx.match(m_lstBasicWords.at(i)->decomposedWord()).hasMatch())
#else
			if (rx.exactMatch(m_lstBasicWords.at(i)->decomposedWord()))
#endif
				return i;
			}
		return -1;
	}

private:
	const TBasicWordList &m_lstBasicWords;
};

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

#ifdef QT_WIDGETS_LIB

CSearchCompleter::CSearchCompleter(const CParsedPhrase &parsedPhrase, QWidget *parentWidget)
	:	QCompleter(parentWidget),
		m_nCompletionFilterMode(SCFME_NORMAL),
		m_pSearchStringListModel(nullptr),
		m_pSoundExFilterModel(nullptr)
{
	m_pSearchStringListModel = new CSearchParsedPhraseListModel(parsedPhrase, this);
	m_pSoundExFilterModel = new CSoundExSearchCompleterFilter(m_pSearchStringListModel, this);

	setWidget(parentWidget);
	setCaseSensitivity(Qt::CaseInsensitive);
	// Note: CompletionMode, CompletionRole, and ModelSorting properties are set in setCompletionFilterMode(), as they depend on the mode:
	setCompletionFilterMode(m_nCompletionFilterMode);
	setModel(m_pSoundExFilterModel);
}

CSearchCompleter::CSearchCompleter(CDictionaryDatabasePtr pDictionary, const QTextEdit &editorWord, QWidget *parentWidget)
	:	QCompleter(parentWidget),
		m_nCompletionFilterMode(SCFME_NORMAL),
		m_pSearchStringListModel(nullptr),
		m_pSoundExFilterModel(nullptr)
{
	m_pSearchStringListModel = new CSearchDictionaryListModel(pDictionary, editorWord, this);
	m_pSoundExFilterModel = new CSoundExSearchCompleterFilter(m_pSearchStringListModel, this);

	setWidget(parentWidget);
	setCaseSensitivity(Qt::CaseInsensitive);
	// Note: CompletionMode, CompletionRole, and ModelSorting properties are set in setCompletionFilterMode(), as they depend on the mode:
	setCompletionFilterMode(m_nCompletionFilterMode);
	setModel(m_pSoundExFilterModel);
}

CSearchCompleter::CSearchCompleter(QWidget *parentWidget)
	:	QCompleter(parentWidget),
		m_nCompletionFilterMode(SCFME_NORMAL),
		m_pSearchStringListModel(nullptr),
		m_pSoundExFilterModel(nullptr)
{
	setWidget(parentWidget);
	setCaseSensitivity(Qt::CaseInsensitive);
}

CSearchCompleter::~CSearchCompleter()
{

}

void CSearchCompleter::setCompletionFilterMode(SEARCH_COMPLETION_FILTER_MODE_ENUM nCompletionFilterMode)
{
	Q_ASSERT(m_pSoundExFilterModel != nullptr);

	switch (nCompletionFilterMode) {
		case SCFME_NORMAL:
			setCompletionMode(QCompleter::PopupCompletion);
			m_pSoundExFilterModel->setSoundExEnabled(false);
			setCompletionRole(Qt::EditRole);
			setModelSorting(QCompleter::CaseInsensitivelySortedModel);
			break;
		case SCFME_UNFILTERED:
			setCompletionMode(QCompleter::UnfilteredPopupCompletion);
			m_pSoundExFilterModel->setSoundExEnabled(false);
			setCompletionRole(Qt::DisplayRole);
			setModelSorting(QCompleter::UnsortedModel);				// We're sorted by the editRole, not the displayRole
			break;
		case SCFME_SOUNDEX:
			setCompletionMode(QCompleter::UnfilteredPopupCompletion);
			m_pSoundExFilterModel->setSoundExEnabled(true);
			setCompletionRole(Qt::DisplayRole);
			setModelSorting(QCompleter::UnsortedModel);				// We're sorted by the editRole, not the displayRole
			break;
	}

	m_nCompletionFilterMode = nCompletionFilterMode;
	if (!m_pSearchStringListModel->isDynamicModel()) {
		QTimer::singleShot(1, m_pSoundExFilterModel, SLOT(en_modelChanged()));		// Force a delayed update for models that don't do auto per-word updates (i.e. static models)
	}
}

void CSearchCompleter::setFilterMatchString()
{
	QString strPrefix = m_pSearchStringListModel->cursorWord();
#if QT_VERSION >= 0x050500
	int nPreRegExp = strPrefix.indexOf(QRegularExpression("[\\[\\]\\*\\?]"));
#else
	int nPreRegExp = strPrefix.indexOf(QRegExp("[\\[\\]\\*\\?]"));
#endif
	if (nPreRegExp != -1) strPrefix = strPrefix.left(nPreRegExp);
	QString strPrefixDecomposed = StringParse::decompose(strPrefix, true);

#ifdef SEARCH_COMPLETER_DEBUG_OUTPUT
	qDebug("SearchCompleter::setFilterMatchString : %s", strPrefix.toUtf8().data());
#endif

	m_strFilterMatchString = strPrefix;
	setCompletionPrefix((completionFilterMode() == SCFME_NORMAL) ? strPrefixDecomposed : strPrefix);
	m_pSoundExFilterModel->setFilterFixedString(strPrefix);
}

void CSearchCompleter::selectFirstMatchString()
{
#ifdef SEARCH_COMPLETER_DEBUG_OUTPUT
	qDebug("SelectFirstMatch: CursorWord: \"%s\"  CurrentCompletion: \"%s\"", m_pSearchStringListModel->cursorWord().toUtf8().data(), currentCompletion().toUtf8().data());
	qDebug("Completion Model Size: %d", completionModel()->rowCount());
#endif

	popup()->clearSelection();

	QModelIndex indexFirstComposedWord = soundExFilterModel()->firstMatchStringIndex(true);
	QModelIndex indexFirstDecomposedWord = soundExFilterModel()->firstMatchStringIndex(false);

	switch (completionFilterMode()) {
		case SCFME_NORMAL:
		case SCFME_SOUNDEX:
			if (indexFirstComposedWord.isValid()) {
				int nCompCount = completionModel()->rowCount();
				for (int nComp = 0; nComp < nCompCount; ++nComp) {
					QModelIndex ndxComp = completionModel()->index(nComp, 0);
					if (ndxComp.data(Qt::DisplayRole).toString().compare(indexFirstComposedWord.data(Qt::DisplayRole).toString()) == 0) {
						popup()->setCurrentIndex(ndxComp);
						popup()->selectionModel()->select(ndxComp, QItemSelectionModel::Select);
						break;
					}
				}
			} else if (indexFirstDecomposedWord.isValid()) {
				int nCompCount = completionModel()->rowCount();
				for (int nComp = 0; nComp < nCompCount; ++nComp) {
					QModelIndex ndxComp = completionModel()->index(nComp, 0);
					if (ndxComp.data(Qt::EditRole).toString().compare(indexFirstDecomposedWord.data(Qt::EditRole).toString()) == 0) {
						if (completionFilterMode() == SCFME_SOUNDEX) {
							setCompletionPrefix(indexFirstDecomposedWord.data(Qt::DisplayRole).toString());		// Force assert a selection prefix or else it won't select it in this mode
						}
						popup()->setCurrentIndex(ndxComp);
						popup()->selectionModel()->select(ndxComp, QItemSelectionModel::Select);
						break;
					}
				}
			}
			break;
		case SCFME_UNFILTERED:
			if (indexFirstComposedWord.isValid()) {
				popup()->setCurrentIndex(indexFirstComposedWord);
			} else if (indexFirstDecomposedWord.isValid()) {
				int nCompCount = completionModel()->rowCount();
				for (int nComp = 0; nComp < nCompCount; ++nComp) {
					QModelIndex ndxComp = completionModel()->index(nComp, 0);
					if (ndxComp.data(Qt::EditRole).toString().compare(indexFirstDecomposedWord.data(Qt::EditRole).toString()) == 0) {
						setCompletionPrefix(indexFirstDecomposedWord.data(Qt::DisplayRole).toString());		// Force assert a selection prefix or else it won't select it in this mode
						popup()->setCurrentIndex(ndxComp);
						popup()->selectionModel()->select(ndxComp, QItemSelectionModel::Select);
						break;
					}
				}
			}
			break;
	}
}

void CSearchCompleter::setWordsFromPhrase(bool bForceUpdate)
{
	Q_ASSERT(m_pSearchStringListModel != nullptr);
	m_pSearchStringListModel->setWordsFromPhrase(bForceUpdate);
}

#endif

// ============================================================================


// ============================================================================

CSoundExSearchCompleterFilter::CSoundExSearchCompleterFilter(CSearchStringListModel *pSearchStringListModel, QObject *parent)
	:	QAbstractItemModel(parent),
		m_bSoundExEnabled(true),
		m_nFirstComposedMatchStringIndex(-1),
		m_nFirstDecomposedMatchStringIndex(-1),
		m_pSearchStringListModel(pSearchStringListModel)
{
	Q_ASSERT(m_pSearchStringListModel != nullptr);
	connect(m_pSearchStringListModel, SIGNAL(modelChanged()), this, SLOT(en_modelChanged()), Qt::DirectConnection);

	connect(m_pSearchStringListModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(en_dataChanged(const QModelIndex &, const QModelIndex &)));
	connect(m_pSearchStringListModel, SIGNAL(layoutAboutToBeChanged()), this, SIGNAL(layoutAboutToBeChanged()));
	connect(m_pSearchStringListModel, SIGNAL(layoutChanged()), this, SIGNAL(layoutChanged()));
}

CSoundExSearchCompleterFilter::~CSoundExSearchCompleterFilter()
{

}

int CSoundExSearchCompleterFilter::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return 0;

	return ((m_bSoundExEnabled && !m_strFilterFixedString.isEmpty()) ? m_lstMatchedIndexes.size() : m_pSearchStringListModel->rowCount()) ;
}

int CSoundExSearchCompleterFilter::columnCount(const QModelIndex &parent) const
{
	return parent.isValid() ? 0 : 1;
}

bool CSoundExSearchCompleterFilter::hasChildren(const QModelIndex & parent) const
{
	return parent.isValid() ? false : (rowCount() > 0);
}

QModelIndex CSoundExSearchCompleterFilter::parent(const QModelIndex & index) const
{
	Q_UNUSED(index);
	return QModelIndex();
}

QModelIndex	CSoundExSearchCompleterFilter::index(int row, int column, const QModelIndex & parent) const
{
	return hasIndex(row, column, parent) ? createIndex(row, column) : QModelIndex();
}

QModelIndex CSoundExSearchCompleterFilter::mapFromSource(const QModelIndex &sourceIndex) const
{
	if (!sourceIndex.isValid()) return sourceIndex;

	if (sourceIndex.column() == 0) {
		for (int nRow = 0; nRow < m_lstMatchedIndexes.size(); ++nRow) {
			if (m_lstMatchedIndexes.at(nRow) == sourceIndex.row()) return createIndex(nRow, 0);
		}
	}
	return QModelIndex();
}

QModelIndex CSoundExSearchCompleterFilter::mapToSource(const QModelIndex &proxyIndex) const
{
	if (!proxyIndex.isValid()) return proxyIndex;

	if ((proxyIndex.column() == 0) && (proxyIndex.row() >= 0) && (proxyIndex.row() < m_lstMatchedIndexes.size())) {
//		return createIndex(m_lstMatchedIndexes.at(proxyIndex.row()), 0);
		return m_pSearchStringListModel->index(proxyIndex.row());
	}
	return QModelIndex();
}

QVariant CSoundExSearchCompleterFilter::data(const QModelIndex &index, int role) const
{
	if (!index.isValid()) return QVariant();
	if (m_strFilterFixedString.isEmpty() || !m_bSoundExEnabled) {
		return m_pSearchStringListModel->data(m_pSearchStringListModel->index(index.row()), role);
	}
	return m_pSearchStringListModel->data(m_pSearchStringListModel->index(m_lstMatchedIndexes.at(index.row())), role);
}

bool CSoundExSearchCompleterFilter::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (!index.isValid()) return false;
	if (m_strFilterFixedString.isEmpty() || !m_bSoundExEnabled) {
		return m_pSearchStringListModel->setData(m_pSearchStringListModel->index(index.row()), value, role);
	}
	return m_pSearchStringListModel->setData(m_pSearchStringListModel->index(m_lstMatchedIndexes.at(index.row())), value, role);
}

void CSoundExSearchCompleterFilter::setFilterFixedString(const QString &strPattern)
{
#ifdef SEARCH_COMPLETER_DEBUG_OUTPUT
	qDebug("SoundExSearchCompleter::setFilterFixedString : \"%s\" => \"%s\"", m_strFilterFixedString.toUtf8().data(), strPattern.toUtf8().data());
#endif

	bool bNeedUpdate = (m_strFilterFixedString.compare(strPattern) != 0);
	m_strFilterFixedString = strPattern;
	if (bNeedUpdate) {
		if (m_pSearchStringListModel->isDynamicModel()) {
			// This automatically updates our model:
			m_pSearchStringListModel->setWordsFromPhrase(true);		// Must force a rebuild on our dynamic models so our indexes will be correct
		} else {
			updateModel(m_bSoundExEnabled);		// No need to update our model if SoundEx isn't enabled -- the SearchListModel's reset will cause a modelChange that will update it
		}
	}
}

QModelIndex CSoundExSearchCompleterFilter::firstMatchStringIndex(bool bComposed) const
{
	int nIndex = (bComposed ? m_nFirstComposedMatchStringIndex : m_nFirstDecomposedMatchStringIndex);

	if (nIndex != -1) {
		if (m_strFilterFixedString.isEmpty() || !m_bSoundExEnabled) {
			return m_pSearchStringListModel->index(nIndex, 0);
		} else {
			return index(nIndex, 0);
		}
	}
	return QModelIndex();
}

void CSoundExSearchCompleterFilter::en_modelChanged()
{
#ifdef SEARCH_COMPLETER_DEBUG_OUTPUT
	qDebug("SoundExSearchCompleter::modelChanged");
#endif

	CBusyCursor iAmBusy(nullptr);

	int nCount = m_pSearchStringListModel->rowCount();
	m_mapSoundEx.clear();
	for (int nRow = 0; nRow < nCount; ++nRow) {
		QModelIndex ndx = m_pSearchStringListModel->index(nRow);
		if (m_bSoundExEnabled) m_mapSoundEx[ndx.data(CSearchStringListModel::SOUNDEX_ENTRY_ROLE).toString()].append(nRow);
	}

	updateModel(true);					// Always reset our model when base model resets
}

void CSoundExSearchCompleterFilter::updateModel(bool bResetModel)
{
#ifdef SEARCH_COMPLETER_DEBUG_OUTPUT
	qDebug("SoundExSearchCompleter::updateModel");
#endif

	if (bResetModel) beginResetModel();

	// Note: For Dynamic Models, like Search Phrases, this function can ONLY be called
	//		immediately after setWordsFromPhrase() is called on the SearchStringListModel,
	//		or from the en_modelChanged() slot triggered by the actual SearchStringListModel's
	//		setWordsFromPhrase() function, as the BasicWordsList is only valid immediately
	//		after the model is updated, since it contains references and pointers to
	//		transient data in order to prevent extraneous copying of strings.  On
	//		static models, like the Dictionary, it's safe to call it as needed:

	TBasicWordHelper lstBasicWords(m_pSearchStringListModel->basicWordsList());

	m_lstMatchedIndexes.clear();
	m_nFirstComposedMatchStringIndex = -1;
	m_nFirstDecomposedMatchStringIndex = -1;
	QString strDecomposedFilterString = StringParse::decompose(m_strFilterFixedString, true);
	if (!m_strFilterFixedString.isEmpty()) {
#if QT_VERSION >= 0x050F00
		QRegularExpression expPrefix(QRegularExpression::wildcardToRegularExpression(strDecomposedFilterString + "*"), QRegularExpression::CaseInsensitiveOption);
#else
		QRegExp expPrefix(strDecomposedFilterString + "*", Qt::CaseInsensitive, QRegExp::Wildcard);
#endif

		if (m_bSoundExEnabled) {
			QString strSoundEx = m_pSearchStringListModel->soundEx(strDecomposedFilterString, false);

#ifdef SEARCH_COMPLETER_DEBUG_OUTPUT
			qDebug("SoundEx: \"%s\" => %s", m_strFilterFixedString.toUtf8().data(), strSoundEx.toUtf8().data());
#endif

			int nFirstWord = lstBasicWords.indexOf_decomposedWord(expPrefix);
			int nLastWord = ((nFirstWord != -1) ? lstBasicWords.lastIndexOf_decomposedWord(expPrefix) : -1);
			int nNumWords = ((nFirstWord != -1) ? (nLastWord - nFirstWord + 1) : 0);

#ifdef SEARCH_COMPLETER_DEBUG_OUTPUT
			qDebug("Prefix: \"%s\"  expPrefix: \"%s\"  nFirst: %d  nLast: %d", strDecomposedFilterString.toUtf8().data(), expPrefix.pattern().toUtf8().data(), nFirstWord, nLastWord);
#endif

			const QList<int> &mapSoundEx = m_mapSoundEx[strSoundEx];

			m_nFirstDecomposedMatchStringIndex = nFirstWord;		// Temporarily set first word index to our decomposed list index.  After sorting, we'll find it's new location and change it
			m_nFirstComposedMatchStringIndex = lstBasicWords.indexOf_renderedWord(m_strFilterFixedString, ((m_nFirstDecomposedMatchStringIndex != -1) ? m_nFirstDecomposedMatchStringIndex : 0));

			QList<int> lstMatches;
			lstMatches.reserve(mapSoundEx.size() + nNumWords);
			lstMatches.append(m_mapSoundEx[strSoundEx]);
			while (nFirstWord <= nLastWord) {
				lstMatches.append(nFirstWord);
				++nFirstWord;
			}

			std::sort(lstMatches.begin(), lstMatches.end());

			// Remove Duplicates:
			m_lstMatchedIndexes.reserve(lstMatches.size());
			int nLastValue = -1;
			for (int nRow = 0; nRow < lstMatches.size(); ++nRow) {
				if (lstMatches.at(nRow) != nLastValue) m_lstMatchedIndexes.append(lstMatches.at(nRow));
				nLastValue = lstMatches.at(nRow);
			}

			// Find our translated first word RegExp match:
			bool bFoundComposed = (m_nFirstComposedMatchStringIndex == -1);
			bool bFoundDecomposed = (m_nFirstDecomposedMatchStringIndex == -1);
			if ((!bFoundComposed) || (!bFoundDecomposed)) {
				for (int nRow = 0; nRow < m_lstMatchedIndexes.size(); ++nRow) {
					if (!bFoundComposed) {
						if (m_lstMatchedIndexes.at(nRow) == m_nFirstComposedMatchStringIndex) {
							m_nFirstComposedMatchStringIndex = nRow;
							bFoundComposed = true;
						}
					}
					if (!bFoundDecomposed) {
						if (m_lstMatchedIndexes.at(nRow) == m_nFirstDecomposedMatchStringIndex) {
							m_nFirstDecomposedMatchStringIndex = nRow;
							bFoundDecomposed = true;
						}
					}
					if (bFoundComposed && bFoundDecomposed) break;
				}
			}
		} else {
			m_nFirstDecomposedMatchStringIndex = lstBasicWords.indexOf_decomposedWord(expPrefix);
			m_nFirstComposedMatchStringIndex = lstBasicWords.indexOf_renderedWord(m_strFilterFixedString, ((m_nFirstDecomposedMatchStringIndex != -1) ? m_nFirstDecomposedMatchStringIndex : 0));

#ifdef SEARCH_COMPLETER_DEBUG_OUTPUT
			qDebug("Prefix: \"%s\"  expPrefix: \"%s\"  nFirstMatch: %d", strDecomposedFilterString.toUtf8().data(), expPrefix.pattern().toUtf8().data(), m_nFirstDecomposedMatchStringIndex);
#endif
		}
	}

	if (bResetModel) endResetModel();

	// For some reason, QCompleter doesn't respond to begin/end ModelReset, but does for dataChanged.
	//		Without the following, the model data gets completely out of sync in the QCompleter... Go figure...
	if (bResetModel) emit dataChanged(QModelIndex(), QModelIndex());
}

void CSoundExSearchCompleterFilter::en_dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
#ifdef SEARCH_COMPLETER_DEBUG_OUTPUT
	qDebug("SoundExSearchCompleter::en_dataChanged");
#endif

	if ((!topLeft.isValid()) || (!bottomRight.isValid())) {
		emit dataChanged(topLeft, bottomRight);			// If either node is invalid, assume it's some special reset all case, pass-on as-is
		return;
	}
	if (m_lstMatchedIndexes.size() == 0) {				// If our filter is empty, pass on unchanged
		emit dataChanged(topLeft, bottomRight);
		return;
	}

	int nRowFirst = 0;
	int nRowLast = 0;
	for (int nRow = 0; nRow < m_lstMatchedIndexes.size(); ++ nRow) {
		if (m_lstMatchedIndexes.at(nRow) < topLeft.row()) {
			nRowFirst = nRow;
			nRowLast = nRow;
		} else {
			if (m_lstMatchedIndexes.at(nRow) < bottomRight.row()) {
				nRowLast = nRow;
			}
		}
	}
	emit dataChanged(createIndex(nRowFirst, 0), createIndex(nRowLast, 0));
}

// ============================================================================

#if QT_VERSION < 0x050000

bool CComposingCompleter::eventFilter(QObject *obj, QEvent *ev)
{
	// The act of popping our completer, will cause the inputContext to
	//		shift focus from the editor to the popup and after dismissing the
	//		popup, it doesn't go back to the editor.  So, since we are eating
	//		FocusOut events in the popup, push the inputContext focus back to
	//		the editor when we "focus out".  It's our focusProxy anyway:
	if ((ev->type() == QEvent::FocusOut) && (obj == widget())) {
		if ((popup()) && (popup()->isVisible())) {
			QInputContext *pInputContext = popup()->inputContext();
			if (pInputContext) pInputContext->setFocusWidget(popup());
		}
	}

	return QCompleter::eventFilter(obj, ev);
}

#endif

// ============================================================================

#ifdef QT_WIDGETS_LIB

CStrongsDictionarySearchCompleter::CStrongsDictionarySearchCompleter(CDictionaryDatabasePtr pDictionary, const QTextEdit &editorWord, QWidget *parentWidget)
	:	SearchCompleter_t(parentWidget),
		m_pStrongsListModel(nullptr)
{
	m_pStrongsListModel = new CSearchStrongsDictionaryListModel(pDictionary, editorWord, this);
	setModel(m_pStrongsListModel);
#if QT_VERSION >= 0x050200		// Filter Mode was introduced in Qt 5.2
	setFilterMode(Qt::MatchStartsWith);
#endif
}

void CStrongsDictionarySearchCompleter::selectFirstMatchString()
{
}

void CStrongsDictionarySearchCompleter::setFilterMatchString()
{
	Q_ASSERT(m_pStrongsListModel != nullptr);
	m_strFilterMatchString = m_pStrongsListModel->cursorWord();
	setCompletionPrefix(m_strFilterMatchString);
}

void CStrongsDictionarySearchCompleter::setWordsFromPhrase(bool bForceUpdate)
{
	Q_UNUSED(bForceUpdate);
}

#endif

// ============================================================================
