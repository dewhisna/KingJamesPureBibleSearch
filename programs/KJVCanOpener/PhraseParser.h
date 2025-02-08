/****************************************************************************
**
** Copyright (C) 2012-2025 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef PHRASE_PARSER_H
#define PHRASE_PARSER_H

#include "dbstruct.h"

#include <QFlags>
#include <QString>
#include <QStringList>
#include <QColor>
#include <QList>
#include <QSharedPointer>

// ============================================================================

// Forward declarations:
class CParsedPhrase;
class QTextCursor;
class CPhraseCursor;

// ============================================================================

class CSubPhrase {
public:
	CSubPhrase();
	CSubPhrase(const CSubPhrase &aSrc);
	~CSubPhrase();
	CSubPhrase &operator =(const CSubPhrase &aSrc);

	int GetMatchLevel() const;
	int GetCursorMatchLevel() const;
	QString GetCursorWord() const;
	int GetCursorWordPos() const;
	QString phrase() const;						// Return reconstituted phrase
	QString phraseRaw() const;					// Return reconstituted phrase without punctuation or regexp symbols
	QString phraseToSpeak() const;				// Return reconstituted phrase to speak (i.e. no TransChangeAdded symbols)
	int phraseSize() const;						// Return number of words in reconstituted phrase
	int phraseRawSize() const;					// Return number of words in reconstituted raw phrase
	int phraseToSpeakSize() const;				// Return number of words in reconstituted phrase to speak
	QStringList phraseWords() const;			// Return reconstituted phrase words
	QStringList phraseWordsRaw() const;			// Return reconstituted raw phrase words
	QStringList phraseWordsToSpeak() const;		// Return reconstituted phrase to speak words

	bool isCompleteMatch() const;
	unsigned int GetNumberOfMatches() const;

	void ClearPhase();
	void ParsePhrase(const QString &strPhrase);
	void ParsePhrase(const QStringList &lstPhrase);
	void AppendPhrase(const QString &strPhrase);
	void AppendPhrase(const QStringList &lstPhrase);

private:
	friend class CParsedPhrase;

	int m_nLevel;				// Level of the search (Number of words matched).  This is the offset value for entries in m_lstMatchMapping (at 0 mapping is ALL words) (Set by FindWords())
	TNormalizedIndexList m_lstMatchMapping;	// Mapping for entire search -- This is the search result, but with each entry offset by the search level (Set by FindWords())
	int m_nCursorLevel;			// Matching level at cursor
	TConcordanceList m_lstNextWords;	// List of words mapping next for this phrase (Set by FindWords()) (Stored as decomposed-normalized strings to help sorting order in auto-completer)

	QStringList m_lstWords;		// Fully Parsed Word list.  Blank entries only at first or last entry to indicate an insertion point. (Filled by ParsePhrase())
	int m_nCursorWord;			// Index in m_lstWords where the cursor is at -- If insertion point is in the middle of two words, Cursor will be at the left word (Set by ParsePhrase())
	QString m_strCursorWord;	// Word at the cursor point between the left and right hand halves (Set by ParsePhrase())
};

typedef QList< QSharedPointer<CSubPhrase> > TSubPhraseList;

// ============================================================================

// Forward declaration:
class CParsedPhrasePtr;

class CParsedPhrase
{
public:
	CParsedPhrase(CBibleDatabasePtr pBibleDatabase = CBibleDatabasePtr(), bool bCaseSensitive = false, bool bAccentSensitive = false, bool bExclude = false);
	CParsedPhrase(const CParsedPhrase &aSrc);
	virtual ~CParsedPhrase();

	void registerSmartPointer(CParsedPhrasePtr *pSmartPtr) const
	{
		if (pSmartPtr != nullptr) m_lstSmartPointers.append(pSmartPtr);
	}
	void unregisterSmartPointer(CParsedPhrasePtr *pSmartPtr) const
	{
		m_lstSmartPointers.removeAll(pSmartPtr);
	}

	CParsedPhrase & operator=(const CParsedPhrase &aSrc);

	bool hasChanged() const { return m_bHasChanged; }
	void setHasChanged(bool bHasChanged) const { m_bHasChanged = bHasChanged; }		// Mutable flag we can set on objects to detect changing/updating in other threads

	// ------- Helpers functions for CSearchCompleter and CSearchStringListModel usage:
	const TConcordanceList &nextWordsList() const;
	bool atEndOfSubPhrase() const;						// True if the cursor is at the end of the active subPhrase

	// ------- Helpers functions for data maintained by controlling CKJVCanOpener class to
	//			use for maintaining statistics about this phrase in context with others and
	//			to build Search Results Verse Lists, do highlighting, etc.
	inline bool isDuplicate() const { return m_bIsDuplicate; }
	inline void setIsDuplicate(bool bIsDuplicate) const { m_bIsDuplicate = bIsDuplicate; }
	inline bool isDisabled() const { return m_bIsDisabled; }
	inline void setIsDisabled(bool bIsDisabled) const { m_bIsDisabled = bIsDisabled; }
	inline int GetContributingNumberOfMatches() const { return m_lstScopedPhraseTagResults.size(); }
	inline const TPhraseTagList &GetScopedPhraseTagSearchResults() const { return m_lstScopedPhraseTagResults; }			// Returned as reference so we don't have to keep copying
	inline TPhraseTagList &GetScopedPhraseTagSearchResultsNonConst() const { return m_lstScopedPhraseTagResults; }			// Non-const version used by VerseListModel for setting
	inline void ClearScopedPhraseTagSearchResults() const { m_lstScopedPhraseTagResults.clear(); }
	inline int GetNumberOfMatchesWithin() const { return m_lstWithinPhraseTagResults.size(); }
	inline const TPhraseTagList &GetWithinPhraseTagSearchResults() const { return m_lstWithinPhraseTagResults; }			// Returned as reference so we don't have to keep copying
	inline TPhraseTagList &GetWithinPhraseTagSearchResultsNonConst() const { return m_lstWithinPhraseTagResults; }			// Non-const version used by VerseListModel for setting
	inline void ClearWithinPhraseTagSearchResults() const { m_lstWithinPhraseTagResults.clear(); }
	// -------
	bool isCompleteMatch() const;
	unsigned int GetNumberOfMatches() const;

	const TPhraseTagList &GetPhraseTagSearchResults() const;		// Returned as reference so we don't have to keep copying
	QStringList GetMatchingPhrases() const;

	QString GetCursorWord() const;				// CursorWord for entire composite of all subPhrases
	int GetCursorWordPos() const;				// CursorWordPos for entire composite of all subPhrases -- includes tail insertion point for each subPhrase at each '|'
	QString phrase() const;						// Return reconstituted phrase
	QString phraseRaw() const;					// Return reconstituted phrase without punctuation or regexp symbols
	QString phraseToSpeak() const;				// Return reconstituted phrase to speak (i.e. no TransChangeAdded symbols)
	const QStringList phraseWords() const;		// Return reconstituted phrase words
	const QStringList phraseWordsRaw() const;	// Return reconstituted raw phrase words
	const QStringList phraseWordsToSpeak() const;	// Return reconstituted phrase to speak words

	int subPhraseCount() const { return m_lstSubPhrases.size(); }
	int currentSubPhrase() const { return m_nActiveSubPhrase; }
	const CSubPhrase *subPhrase(int nIndex) const
	{
		Q_ASSERT((nIndex >= 0) && (nIndex < m_lstSubPhrases.size()));
		return m_lstSubPhrases.at(nIndex).data();
	}
	QSharedPointer<CSubPhrase> primarySubPhrase() const
	{
		return m_pPrimarySubPhrase;
	}

	virtual void ParsePhrase(const QTextCursor &curInsert, bool bFindWords = true);		// Parses the phrase in the editor.  Sets m_lstWords and m_nCursorWord (Clears word cache and bFindWords determines if we FindWords() for our BibleDatabase)
	virtual void ParsePhrase(const QString &strPhrase, bool bFindWords = false);		// Parses a fixed phrase (Clears word cache and does NOT call FindWords())
	virtual void ParsePhrase(const QStringList &lstPhrase, bool bFindWords = false);	// Parses a fixed phrase already divided into words (like getSelectedPhrases from CTextNavigator) (Clears word cache and does NOT call FindWords())

	virtual bool isCaseSensitive() const { return m_bCaseSensitive; }
	virtual void setCaseSensitive(bool bCaseSensitive) { m_bCaseSensitive = bCaseSensitive; }

	virtual bool isAccentSensitive() const { return m_bAccentSensitive; }
	virtual void setAccentSensitive(bool bAccentSensitive) { m_bAccentSensitive = bAccentSensitive; }

	virtual bool isExcluded() const { return m_bExclude; }
	virtual void setExclude(bool bExclude) { m_bExclude = bExclude; }

	bool operator==(const CParsedPhrase &src) const
	{
		return ((m_bCaseSensitive == src.m_bCaseSensitive) &&
				(m_bAccentSensitive == src.m_bAccentSensitive) &&
				(m_bExclude == src.m_bExclude) &&
				(phrase().compare(src.phrase(), Qt::CaseSensitive) == 0));
	}
	bool operator!=(const CParsedPhrase &src) const
	{
		return (!(operator==(src)));
	}

	bool operator==(const CPhraseEntry &src) const
	{
		return ((m_bCaseSensitive == src.caseSensitive()) &&
				(m_bAccentSensitive == src.accentSensitive()) &&
				(m_bExclude == src.isExcluded()) &&
				(phrase().compare(src.text(), Qt::CaseSensitive) == 0));
	}
	bool operator!=(const CPhraseEntry &src) const
	{
		return (!(operator==(src)));
	}

	virtual void setFromPhraseEntry(const CPhraseEntry &aPhraseEntry, bool bFindWords)
	{
		if (*this != aPhraseEntry) {		// Only set the phrase if it's different to avoid repeated FindWords() calls
			setCaseSensitive(aPhraseEntry.caseSensitive());
			setAccentSensitive(aPhraseEntry.accentSensitive());
			setExclude(aPhraseEntry.isExcluded());
			setIsDisabled(aPhraseEntry.isDisabled());
			ParsePhrase(aPhraseEntry.text(), bFindWords);
		}
	}

	void UpdateCompleter(const QTextCursor &curInsert);
	QTextCursor insertCompletion(const QTextCursor &curInsert, const QString& completion);
	void clearCache() const;

	virtual void FindWords();						// Calls FindWords(subPhrase) on each subphrase
	virtual void ResumeFindWords();					// Resumes the FindWords() logic without clearing current results -- used for manually adding new words to the end of the phrases only, as an optimization!

	inline const CBibleDatabase *bibleDatabase() const { return m_pBibleDatabase.data(); }

protected:
	QSharedPointer<CSubPhrase> attachSubPhrase(CSubPhrase *pSubPhrase)				// Take ownership of externally created CSubPhrase
	{
		return attachSubPhrase(QSharedPointer<CSubPhrase>(pSubPhrase));
	}
	QSharedPointer<CSubPhrase> attachSubPhrase(const QSharedPointer<CSubPhrase> &pSubPhrase)
	{
		if (!m_pBibleDatabase.isNull()) pSubPhrase->m_lstNextWords = m_pBibleDatabase->concordanceWordList();
		m_lstSubPhrases.append(QSharedPointer<CSubPhrase>(pSubPhrase));
		return m_lstSubPhrases.last();
	}

	// This one needs to be private because if the subPhrase referenced isn't
	//	owned by this ParsePhrase object, then you can't get the results back!
	//	TODO : Consider moving this function to CSubPhrase.  It could almost be
	//	static now except that it needs m_pBibleDatabase.  CSubPhrase doesn't
	//	currently have m_pBibleDatabase but maybe it should?
	void FindWords(CSubPhrase &subPhrase, bool bResume = false);	// Uses m_lstWords and m_nCursorWord to populate m_lstNextWords, m_lstMatchMapping, and m_nLevel

	CBibleDatabasePtr m_pBibleDatabase;
	mutable TPhraseTagList m_cache_lstPhraseTagResults;		// Cached Denormalized Search Results converted to phrase tags (Set on call to GetPhraseTagSearchResults, cleared on ClearCache)
	// -------
	mutable bool m_bIsDuplicate;							// Indicates this phrase is exact duplicate of another phrase.  Set/Cleared by parent phraseChanged logic.
	mutable bool m_bIsDisabled;								// Indicates this phrase is disabled.  Set/Cleared by parent phraseChanged logic
	mutable TPhraseTagList m_lstScopedPhraseTagResults;		// List of Denormalized Search Results from Scope.  Set/Cleared by parent phraseChanged logic and buildScopedResultsInParsedPhrases on VerseListModel.  The size of this list is the GetContributingNumberOfMatches
	mutable TPhraseTagList m_lstWithinPhraseTagResults;		// List of Denormalized Search Results from within Selected Search Documents (but not scoped with other phrases).  Set/Cleared by parent phraseChanged logic and buildScopedResultsInParsedPhrases on VerseListModel.  The size of this list is the GetNumberOfMatchesWithin
	// -------
	bool m_bCaseSensitive;
	bool m_bAccentSensitive;
	bool m_bExclude;

	int m_nActiveSubPhrase;
	TSubPhraseList m_lstSubPhrases;
	QSharedPointer<CSubPhrase> m_pPrimarySubPhrase;			// Kept as the first and foremost SubPhrase in m_lstSubPhrases

	mutable bool m_bHasChanged;								// Flag to detect text/setting changed, set by CPhraseLineEdit child object via phraseChanged() signal, cleared on thread copy operation -- used for multithreading phrase change detection
	mutable QList< CParsedPhrasePtr * > m_lstSmartPointers;	// Smart pointers to work like QPointer for non-QObject
};

class CParsedPhrasePtr
{
public:
	CParsedPhrasePtr()
		:	m_pParsedPhrase(nullptr)
	{
	}
	CParsedPhrasePtr(const CParsedPhrasePtr &aSrc)
		:	m_pParsedPhrase(aSrc.m_pParsedPhrase)
	{
		if (m_pParsedPhrase != nullptr) m_pParsedPhrase->registerSmartPointer(this);
	}
	CParsedPhrasePtr(const CParsedPhrase *pParsedPhrase)
		:	m_pParsedPhrase(pParsedPhrase)
	{
		if (m_pParsedPhrase != nullptr) m_pParsedPhrase->registerSmartPointer(this);
	}
	~CParsedPhrasePtr()
	{
		if (m_pParsedPhrase != nullptr) m_pParsedPhrase->unregisterSmartPointer(this);
	}

	CParsedPhrasePtr & operator=(const CParsedPhrasePtr &aSrc)
	{
		// Need to have explicit operator=() since we have explicit copy constructor.
		//	Plus, we need to diddle the smart pointer registration:
		if (m_pParsedPhrase != nullptr) m_pParsedPhrase->unregisterSmartPointer(this);
		m_pParsedPhrase = aSrc.m_pParsedPhrase;
		if (m_pParsedPhrase != nullptr) m_pParsedPhrase->registerSmartPointer(this);
		return *this;
	}

	inline const CParsedPhrase &operator*() const { return *m_pParsedPhrase; }
	inline operator const CParsedPhrase*() const { return m_pParsedPhrase; }
	inline const CParsedPhrase *operator->() const { return m_pParsedPhrase; }
	inline const CParsedPhrase *data() const { return m_pParsedPhrase; }
	inline bool isNull() const { return !m_pParsedPhrase; }

protected:
	friend class CParsedPhrase;
	void clear()
	{
		m_pParsedPhrase = nullptr;
	}

private:
	const CParsedPhrase *m_pParsedPhrase;
};

typedef QList< CParsedPhrasePtr > TParsedPhrasesList;
typedef QList< QSharedPointer<CParsedPhrase> > TSharedParsedPhrasesList;

class CSelectedPhrase
{
public:
	CSelectedPhrase(CBibleDatabasePtr pBibleDatabase, const TPhraseTag &aTag, const QString &strPhrase, bool bCaseSensitive = false, bool bAccentSensitive = false, bool bExclude = false)
		:	m_ParsedPhrase(pBibleDatabase, bCaseSensitive, bAccentSensitive, bExclude),
		m_Tag(aTag)
	{
		m_ParsedPhrase.ParsePhrase(strPhrase);
	}

	inline const CParsedPhrase &phrase() const { return m_ParsedPhrase; }
	inline const TPhraseTag &tag() const { return m_Tag; }

	bool operator==(const CSelectedPhrase &src) const
	{
		return (m_Tag == src.m_Tag);
	}
	bool operator!=(const CSelectedPhrase &src) const
	{
		return (m_Tag != src.m_Tag);
	}

private:
	CParsedPhrase m_ParsedPhrase;
	TPhraseTag m_Tag;
};

class CSelectionPhraseTagList : public TPhraseTagList
{
public:
	CSelectionPhraseTagList()
		:	TPhraseTagList()
	{ }

	CSelectionPhraseTagList(const CSelectionPhraseTagList &aTagList)
		:	TPhraseTagList(aTagList)
	{ }

	CSelectionPhraseTagList(const TPhraseTagList &aTagList)
		:	TPhraseTagList(aTagList)
	{ }

	bool haveSelection() const {
		if (size() == 0) return false;
		for (int ndx = 0; ndx < size(); ++ndx) {
			if (at(ndx).haveSelection()) return true;
		}
		return false;
	}
	TPhraseTag primarySelection() const
	{
		if (haveSelection()) {
			for (int ndx = 0; ndx < size(); ++ndx) {
				if (at(ndx).haveSelection()) return at(ndx);
			}
		}
		// If we don't have a real selection (i.e. something with words), pick
		//		the first entry, which will contain a relIndex for a location:
		if (size() > 0) return at(0);
		return TPhraseTag();
	}
};

class  CSelectedPhraseList : public QList<CSelectedPhrase>
{
public:
	bool haveSelection() const {
		if (size() == 0) return false;
		for (int ndx = 0; ndx < size(); ++ndx) {
			if (at(ndx).tag().haveSelection()) return true;
		}
		return false;
	}
	const CSelectedPhrase &primarySelectionPhrase() const
	{
		Q_ASSERT(size() != 0);
		Q_ASSERT(haveSelection());
		for (int ndx = 0; ndx < size(); ++ndx) {
			if (at(ndx).tag().haveSelection()) return at(ndx);
		}
		return at(0);
	}
	CSelectionPhraseTagList selection() const
	{
		CSelectionPhraseTagList lstSelection;
		for (int ndx = 0; ndx < size(); ++ndx) {
			lstSelection.append(at(ndx).tag());
		}
		return lstSelection;
	}

	enum PHRASE_COMBINE_MODE_ENUM {
		PCME_SPACE = 0,					// Combine phrases with single space
		PCME_NEWLINE = 1,				// Combine phrases with each on a newline
		PCME_NEWLINE_TWO = 2			// Combine phrases with each on a newline and additional newline between them
	};

	QString phrase(PHRASE_COMBINE_MODE_ENUM nCombineMode = PCME_SPACE) const					// Combined phrase of all selected phrases
	{
		QStringList lstPhrases;
		for (int ndx = 0; ndx < size(); ++ndx) {
			if (!at(ndx).tag().haveSelection()) continue;
			if ((nCombineMode == PCME_NEWLINE) ||
				(nCombineMode == PCME_NEWLINE_TWO)) {
				lstPhrases.append(at(ndx).phrase().phrase() + "\n");
			} else {
				lstPhrases.append(at(ndx).phrase().phrase());
			}
		}

		switch (nCombineMode) {
			case PCME_NEWLINE:
				return lstPhrases.join("");
			case PCME_NEWLINE_TWO:
				return lstPhrases.join("\n");
			case PCME_SPACE:
			default:
				return lstPhrases.join(" ");
		}
	}

	QString phraseRaw(PHRASE_COMBINE_MODE_ENUM nCombineMode = PCME_SPACE) const					// Combined raw phrase of all selected phrases
	{
		QStringList lstPhrases;
		for (int ndx = 0; ndx < size(); ++ndx) {
			if (!at(ndx).tag().haveSelection()) continue;
			if ((nCombineMode == PCME_NEWLINE) ||
				(nCombineMode == PCME_NEWLINE_TWO)) {
				lstPhrases.append(at(ndx).phrase().phraseRaw() + "\n");
			} else {
				lstPhrases.append(at(ndx).phrase().phraseRaw());
			}
		}

		switch (nCombineMode) {
			case PCME_NEWLINE:
				return lstPhrases.join("");
			case PCME_NEWLINE_TWO:
				return lstPhrases.join("\n");
			case PCME_SPACE:
			default:
				return lstPhrases.join(" ");
		}
	}

	QString phraseToSpeak() const																// Combined phrase to speak of all selected phrases
	{
		QStringList lstPhrases;
		for (int ndx = 0; ndx < size(); ++ndx) {
			if (!at(ndx).tag().haveSelection()) continue;
			lstPhrases.append(at(ndx).phrase().phraseToSpeak());
		}
		return lstPhrases.join(" ");
	}
};

// ============================================================================

#endif // PHRASE_PARSER_H
