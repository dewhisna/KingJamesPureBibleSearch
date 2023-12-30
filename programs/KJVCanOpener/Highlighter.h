/****************************************************************************
**
** Copyright (C) 2012-2022 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include "dbstruct.h"

#include <QObject>
#include <QTextCharFormat>
#include <QList>
#include <QString>
#include <map>

// ============================================================================

constexpr int MAX_HIGHLIGHTER_NAME_SIZE = 40;					// Maximum number of characters in Highlighter Names

// ============================================================================

//
// User highlighter data is stored as:
//			Map of BibleDatabaseUUID (Highlighter) -> Map of Versifications -> Map of HighlighterName -> TPhraseTagList
//
//	This way we can find all of the highlightations that are for the specific
//		database being rendered, and then find the highlighter name for which
//		to apply it...
//

struct HighlighterNameSortPredicate {
	bool operator() (const QString &v1, const QString &v2) const;
};

// PhraseTag Highlighter Mapping Types:
typedef std::map<QString, TPhraseTagList, HighlighterNameSortPredicate> THighlighterTagMap;		// Map of HighlighterName to TPhraseTagList (Highlighters are kept in sorted decomposed alphabetical order for overlay order)
typedef std::map<QString, THighlighterTagMap> TVersificationHighlighterTagMap;					// Map of Versification UUID to THighlighterTagMap
typedef std::map<QString, TVersificationHighlighterTagMap> TBibleDBHighlighterTagMap;			// Map of Bible Database UUID to TVersificationHighlighterTagMap

// ============================================================================

// Forward Declarations:
class i_TVerseListModelResults;									// Nasty intermediate class type defintion for CVerseListModel::TVerseListModelResults, but avoids very nasty header interdependency
class CVerseListModel;
class CVerseListItem;
typedef QMap<CRelIndex, CVerseListItem> CVerseMap;				// Redundant with definition in VerseListModel.h, but avoids very nasty header interdependency

// ============================================================================

class CHighlighterPhraseTagFwdItr
{
protected:
	CHighlighterPhraseTagFwdItr(const i_TVerseListModelResults *pvlmResults);			// Takes ownership of i_TVerseListModelResults object
	CHighlighterPhraseTagFwdItr(const TPhraseTagList &lstTags);

public:
	~CHighlighterPhraseTagFwdItr();

public:
	TPhraseTag nextTag();
	bool isEnd() const;

private:
	const i_TVerseListModelResults *m_pvlmResults;
	const TPhraseTagList &m_lstPhraseTags;
	TPhraseTagList m_lstDummyPhraseTags;				// Dummy list used for m_lstPhraseTags when iterating verses

	CVerseMap::const_iterator m_itrVerses;
	TPhraseTagList::const_iterator m_itrTags;

	friend class CBasicHighlighter;
	friend class CSearchResultHighlighter;
	friend class CCursorFollowHighlighter;
	friend class CUserDefinedHighlighter;
};

// ============================================================================

class CBasicHighlighter : public QObject {
	Q_OBJECT
public:
	explicit CBasicHighlighter(QObject *parent = nullptr)
		:	QObject(parent),
			m_bEnabled(true)
	{
	}

	virtual QTextCharFormat doHighlighting(const QTextCharFormat &aFormat, bool bClear) const = 0;
	virtual bool enabled() const { return m_bEnabled; }
	virtual bool intersects(const CBibleDatabase *pBibleDatabase, const TPhraseTag &aTag) const = 0;

	virtual CHighlighterPhraseTagFwdItr getForwardIterator() const = 0;
	virtual bool isEmpty() const = 0;

	virtual bool isContinuous() const { return false; }			// Continuous = no word breaks in highlighting.  Default is false

	virtual QString htmlBegin() const { return QString(); }
	virtual QString htmlEnd() const { return QString(); }

public slots:
	virtual void setEnabled(bool bEnabled = true) { m_bEnabled = bEnabled; }

signals:
	void phraseTagsChanged();
	void charFormatsChanged();

protected:
	bool m_bEnabled;
	Q_DISABLE_COPY(CBasicHighlighter)
};

// ============================================================================

class CSearchResultHighlighter : public CBasicHighlighter
{
	Q_OBJECT
public:
	explicit CSearchResultHighlighter(const CVerseListModel *pVerseListModel, bool bExcludedResults = false, QObject *parent = nullptr);
	CSearchResultHighlighter(const TPhraseTagList &lstPhraseTags, bool bExcludedResults = false, QObject *parent = nullptr);
	virtual ~CSearchResultHighlighter();

	virtual QTextCharFormat doHighlighting(const QTextCharFormat &aFormat, bool bClear) const override;
	virtual bool intersects(const CBibleDatabase *pBibleDatabase, const TPhraseTag &aTag) const override;

	virtual CHighlighterPhraseTagFwdItr getForwardIterator() const override;
	virtual bool isEmpty() const override;

	virtual QString htmlBegin() const override;
	virtual QString htmlEnd() const override;

	bool isExcludedResults() const { return m_bExcludedResults; }

private slots:
	void verseListChanged();
	void verseListModelDestroyed();

private:
	const CVerseListModel *m_pVerseListModel;
	bool m_bExcludedResults;						// True if this highlighter is displaying excluded search results, false if displaying normal search results

	// Guard class to keep me from accidentally accessing non-const functions and
	//		causing unintentional copying, as that can be expensive in large searches:
	class CMyPhraseTags {
	public:
		inline const TPhraseTagList &phraseTags() const { return m_lstPhraseTags; }
		inline void setPhraseTags(const TPhraseTagList &lstPhraseTags) { m_lstPhraseTags = lstPhraseTags; }

	private:
		TPhraseTagList m_lstPhraseTags;				// Tags to highlight
	} m_myPhraseTags;
};

// ============================================================================

class CCursorFollowHighlighter : public CBasicHighlighter
{
	Q_OBJECT
public:
	explicit CCursorFollowHighlighter(const TPhraseTagList &lstPhraseTags = TPhraseTagList(), QObject *parent = nullptr)
		:	CBasicHighlighter(parent)
	{
		m_myPhraseTags.setPhraseTags(lstPhraseTags);
	}
	CCursorFollowHighlighter(const TPhraseTag &aTag, QObject *parent = nullptr)
		:	CBasicHighlighter(parent)
	{
		TPhraseTagList lstTags;
		lstTags.append(aTag);
		m_myPhraseTags.setPhraseTags(lstTags);
	}
	CCursorFollowHighlighter(const CCursorFollowHighlighter &aCursorFollowHighlighter)
		:	CBasicHighlighter(aCursorFollowHighlighter.parent())
	{
		setEnabled(aCursorFollowHighlighter.enabled());
		m_myPhraseTags.setPhraseTags(aCursorFollowHighlighter.m_myPhraseTags.phraseTags());
	}

	virtual QTextCharFormat doHighlighting(const QTextCharFormat &aFormat, bool bClear) const override;
	virtual bool intersects(const CBibleDatabase *pBibleDatabase, const TPhraseTag &aTag) const override;

	virtual CHighlighterPhraseTagFwdItr getForwardIterator() const override;
	virtual bool isEmpty() const override;

	const TPhraseTagList &phraseTags() const;
	void setPhraseTags(const TPhraseTagList &lstPhraseTags);

public slots:
	void clearPhraseTags();

private:
	// Guard class to keep me from accidentally accessing non-const functions and
	//		causing unintentional copying, as that can be expensive in large searches:
	class CMyPhraseTags {
	public:
		inline const TPhraseTagList &phraseTags() const { return m_lstPhraseTags; }
		inline void setPhraseTags(const TPhraseTagList &lstPhraseTags) { m_lstPhraseTags = lstPhraseTags; }

	private:
		TPhraseTagList m_lstPhraseTags;				// Tags to highlight
	} m_myPhraseTags;
};

// ============================================================================

class CUserDefinedHighlighter : public CBasicHighlighter
{
	Q_OBJECT
public:
	explicit CUserDefinedHighlighter(const QString &strUserDefinedHighlighterName, const TPhraseTagList &lstPhraseTags = TPhraseTagList(), QObject *parent = nullptr);
	CUserDefinedHighlighter(const QString &strUserDefinedHighlighterName, const TPhraseTag &aTag, QObject *parent = nullptr);
	CUserDefinedHighlighter(const CUserDefinedHighlighter &aUserDefinedHighlighter);

	virtual QTextCharFormat doHighlighting(const QTextCharFormat &aFormat, bool bClear) const override;
	virtual bool intersects(const CBibleDatabase *pBibleDatabase, const TPhraseTag &aTag) const override;

	virtual CHighlighterPhraseTagFwdItr getForwardIterator() const override;
	virtual bool isEmpty() const override;

	virtual bool isContinuous() const override { return true; }

	virtual QString htmlBegin() const override;
	virtual QString htmlEnd() const override;

	const TPhraseTagList &phraseTags() const;
	void setPhraseTags(const TPhraseTagList &lstPhraseTags);

public slots:
	void clearPhraseTags();

private:
	// Guard class to keep me from accidentally accessing non-const functions and
	//		causing unintentional copying, as that can be expensive in large searches:
	class CMyPhraseTags {
	public:
		inline const TPhraseTagList &phraseTags() const { return m_lstPhraseTags; }
		inline void setPhraseTags(const TPhraseTagList &lstPhraseTags) { m_lstPhraseTags = lstPhraseTags; }

	private:
		TPhraseTagList m_lstPhraseTags;				// Tags to highlight
	} m_myPhraseTags;

	QString m_strUserDefinedHighlighterName;		// Name of User Defined Highlighter to use
};

// ============================================================================

#endif // HIGHLIGHTER_H
