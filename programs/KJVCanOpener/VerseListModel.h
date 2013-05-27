/****************************************************************************
**
** Copyright (C) 2012 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef VERSELISTMODEL_H
#define VERSELISTMODEL_H

#include "dbstruct.h"
#include "PhraseEdit.h"
#include "KJVSearchCriteria.h"
#include "VerseRichifier.h"

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QList>
#include <QMap>
#include <QStringList>
#include <QVariant>
#include <QPair>
#include <QObject>
#include <QSize>

#include <assert.h>

// ============================================================================

class CVerseListItem
{
public:
	explicit CVerseListItem(CBibleDatabasePtr pBibleDatabase = CBibleDatabasePtr(),
							const CRelIndex &ndx = CRelIndex(),
							const unsigned int nPhraseSize = 1)
		:	m_pBibleDatabase(pBibleDatabase),
		  m_ndxRelative(ndx)
	{
		m_ndxRelative.setWord(0);								// Primary index will have a zero word index
		if (nPhraseSize > 0)
			m_lstTags.push_back(TPhraseTag(ndx, nPhraseSize));		// But the corresponding tag will have non-zero word index
	}
	CVerseListItem(CBibleDatabasePtr pBibleDatabase, const TPhraseTag &tag)
		:	m_pBibleDatabase(pBibleDatabase),
			m_ndxRelative(tag.first)
	{
		m_ndxRelative.setWord(0);				// Primary index will have a zero word index
		if (tag.second > 0)
			m_lstTags.push_back(tag);			// But the corresponding tag will have non-zero word index
	}
	~CVerseListItem()
	{ }

	inline QString getHeading() const {
		assert(m_pBibleDatabase.data() != NULL);
		if (m_pBibleDatabase.data() == NULL) return QString();
		QString strHeading;
		if (m_lstTags.size() > 0) strHeading += QString("(%1) ").arg(m_lstTags.size());
		for (int ndx = 0; ndx < m_lstTags.size(); ++ndx) {
			if (ndx == 0) {
				strHeading += m_pBibleDatabase->PassageReferenceText(m_lstTags.at(ndx).first);
			} else {
				strHeading += QString("[%1]").arg(m_lstTags.at(ndx).first.word());
			}
		}
		return strHeading;
	}

	inline QString getToolTip(const TParsedPhrasesList &phrases = TParsedPhrasesList()) const {
		assert(m_pBibleDatabase.data() != NULL);
		if (m_pBibleDatabase.data() == NULL) return QString();
		QString strToolTip;
		strToolTip += m_pBibleDatabase->SearchResultToolTip(getIndex(), RIMASK_BOOK | RIMASK_CHAPTER | RIMASK_VERSE);
		for (int ndx = 0; ndx < phraseTags().size(); ++ndx) {
			const CRelIndex &ndxTag(phraseTags().at(ndx).first);
			if (phraseTags().size() > 1) {
				strToolTip += QString("(%1)[%2] \"%3\" %4 ").arg(ndx+1).arg(ndxTag.word()).arg(getPhrase(ndx)).arg(QObject::tr("is"));
			} else {
				strToolTip += QString("[%1] \"%2\" %3 ").arg(ndxTag.word()).arg(getPhrase(ndx)).arg(QObject::tr("is"));
			}
			strToolTip += m_pBibleDatabase->SearchResultToolTip(ndxTag, RIMASK_WORD);
			for (int ndxPhrase = 0; ndxPhrase < phrases.size(); ++ndxPhrase) {
				const CParsedPhrase *pPhrase = phrases.at(ndxPhrase);
				assert(pPhrase != NULL);
				if (pPhrase == NULL) continue;
				if (pPhrase->GetPhraseTagSearchResults().contains(phraseTags().at(ndx))) {
					strToolTip += "    " + QObject::tr("%1 of %2 of Search Phrase \"%3\" Results in Entire Bible")
										.arg(pPhrase->GetPhraseTagSearchResults().indexOf(phraseTags().at(ndx)) + 1)
										.arg(pPhrase->GetPhraseTagSearchResults().size())
										.arg(pPhrase->phrase()) + "\n";
				}
				if (pPhrase->GetScopedPhraseTagSearchResults().contains(phraseTags().at(ndx))) {
					strToolTip += "    " + QObject::tr("%1 of %2 of Search Phrase \"%3\" Results in Search Scope")
										.arg(pPhrase->GetScopedPhraseTagSearchResults().indexOf(phraseTags().at(ndx)) + 1)
										.arg(pPhrase->GetScopedPhraseTagSearchResults().size())
										.arg(pPhrase->phrase()) + "\n";
				}
			}
		}

		return strToolTip;
	}

	inline bool isSet() const {
		return (m_ndxRelative.isSet());
	}

	inline uint32_t getBook() const { return m_ndxRelative.book(); }		// Book Number (1-n)
	inline uint32_t getChapter() const { return m_ndxRelative.chapter(); }	// Chapter Number within Book (1-n)
	inline uint32_t getVerse() const { return m_ndxRelative.verse(); }		// Verse Number within Chapter (1-n)
	uint32_t getIndexNormalized() const {
		assert(m_pBibleDatabase.data() != NULL);
		if (m_pBibleDatabase.data() == NULL) return 0;
		return m_pBibleDatabase->NormalizeIndex(m_ndxRelative);
	}
	inline uint32_t getIndexDenormalized() const { return m_ndxRelative.index(); }
	inline CRelIndex getIndex() const { return m_ndxRelative; }
	inline unsigned int getPhraseSize(int nTag) const {
		assert((nTag >= 0) && (nTag < m_lstTags.size()));
		if ((nTag < 0) || (nTag >= m_lstTags.size())) return 0;
		return m_lstTags.at(nTag).second;
	}
	inline CRelIndex getPhraseReference(int nTag) const {
		assert((nTag >= 0) && (nTag < m_lstTags.size()));
		if ((nTag < 0) || (nTag >= m_lstTags.size())) return CRelIndex();
		return m_lstTags.at(nTag).first;
	}
	void addPhraseTag(const CRelIndex &ndx, unsigned int nPhraseSize) { m_lstTags.push_back(TPhraseTag(ndx, nPhraseSize)); }
	void addPhraseTag(const TPhraseTag &tag) { m_lstTags.push_back(tag); }
	const TPhraseTagList &phraseTags() const { return m_lstTags; }

	QStringList getWordList(int nTag) const
	{
		assert(m_pBibleDatabase.data() != NULL);
		if (m_pBibleDatabase.data() == NULL) return QStringList();
		assert((nTag >= 0) && (nTag < m_lstTags.size()));
		if ((!isSet()) || (nTag < 0) || (nTag >= m_lstTags.size())) return QStringList();
		QStringList strWords;
		unsigned int nNumWords = m_lstTags.at(nTag).second;
		uint32_t ndxNormal = m_pBibleDatabase->NormalizeIndex(m_lstTags.at(nTag).first);
		while (nNumWords) {
			strWords.push_back(m_pBibleDatabase->wordAtIndex(ndxNormal));
			ndxNormal++;
			nNumWords--;
		}
		return strWords;
	}
	QString getPhrase(int nTag) const
	{
		return getWordList(nTag).join(" ");
	}

	QStringList getVerseAsWordList() const
	{
		assert(m_pBibleDatabase.data() != NULL);
		if (m_pBibleDatabase.data() == NULL) return QStringList();
		if (!isSet()) return QStringList();
		QStringList strWords;
		const CVerseEntry *pVerseEntry = m_pBibleDatabase->verseEntry(CRelIndex(getBook(), getChapter(), getVerse(), 0));
		unsigned int nNumWords = (pVerseEntry ? pVerseEntry->m_nNumWrd : 0);
		uint32_t ndxNormal = getIndexNormalized();
		while (nNumWords) {
			strWords.push_back(m_pBibleDatabase->wordAtIndex(ndxNormal));
			ndxNormal++;
			nNumWords--;
		}
		return strWords;
	}
	QString getVerseVeryPlainText() const		// Very Plain has no punctuation!
	{
		if (!m_strVeryPlainTextCache.isEmpty()) return m_strVeryPlainTextCache;
		m_strVeryPlainTextCache = getVerseAsWordList().join(" ");
		return m_strVeryPlainTextCache;
	}
	QString getVerseRichText() const
	{
		if (!m_strRichTextCache.isEmpty()) return m_strRichTextCache;
		assert(m_pBibleDatabase.data() != NULL);
		if (m_pBibleDatabase.data() == NULL) return QString();
		if (!isSet()) return QString();
		m_strRichTextCache = m_pBibleDatabase->richVerseText(CRelIndex(getBook(), getChapter(), getVerse(), 0), CVerseTextRichifierTags(), false);
		return m_strRichTextCache;
	}

private:
	CBibleDatabasePtr m_pBibleDatabase;
	CRelIndex m_ndxRelative;		// Primary Relative Index (word index == 0)
	TPhraseTagList m_lstTags;		// Phrase Tags to highlight, includes a copy of the Primary (w/word index != 0)
	mutable QString m_strRichTextCache;			// Caches filled in during first fetch
	mutable QString m_strVeryPlainTextCache;
};

Q_DECLARE_METATYPE(CVerseListItem)

typedef QList<CVerseListItem> CVerseList;
extern void sortVerseList(CVerseList &aVerseList, Qt::SortOrder order);

// ============================================================================

class CVerseListModel : public QAbstractItemModel
{
	Q_OBJECT
public:

	enum VERSE_DISPLAY_MODE_ENUM {
		VDME_HEADING = 0,
		VDME_VERYPLAIN = 1,
		VDME_RICHTEXT = 2,
		VDME_COMPLETE = 3
	};

	enum VERSE_TREE_MODE_ENUM {
		VTME_LIST = 0,					// Display as a linear list (like old QListView used to)
		VTME_TREE_BOOKS = 1,			// Tree by Books = Branch verses under Books w/o Chapters
		VTME_TREE_CHAPTERS = 2			// Tree by Chapters = Branch verses under Chapters under Books
	};

	enum VERSE_DATA_ROLES_ENUM {
		VERSE_ENTRY_ROLE = Qt::UserRole + 0,				// Full verse text display mode
		VERSE_HEADING_ROLE = Qt::UserRole + 1,				// Verse heading display text
		TOOLTIP_ROLE = Qt::UserRole + 2,					// Use our own ToolTip role so we can have user click Ctrl-D to see details ToolTip.  Qt::ToolTip will be a message telling them to do that.
		TOOLTIP_PLAINTEXT_ROLE = Qt::UserRole + 3,			// Same as TOOLTIP_ROLE, but as PlainText instead of RichText
		TOOLTIP_NOHEADING_ROLE = Qt::UserRole + 4,			// Same as TOOLTIP_ROLE, but without Verse Reference Heading
		TOOLTIP_NOHEADING_PLAINTEXT_ROLE = Qt::UserRole + 5	// Same as TOOLTIP_PLAINTEXT_ROLE, but without Verse Reference Heading
	};

	CVerseListModel(CBibleDatabasePtr pBibleDatabase, QObject *parent = 0);
	CVerseListModel(CBibleDatabasePtr pBibleDatabase, const CVerseList &verses, QObject *parent = 0);

	inline CBibleDatabasePtr bibleDatabase() const { return m_pBibleDatabase; }

	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

	virtual QModelIndex	index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const;
	virtual QModelIndex parent(const QModelIndex &index) const;

	virtual QVariant data(const QModelIndex &index, int role) const;
	QVariant dataForVerse(const CVerseListItem &aVerse, int role) const;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

	virtual Qt::ItemFlags flags(const QModelIndex &index) const;

	virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
	virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

	virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

	virtual Qt::DropActions supportedDropActions() const;

	QModelIndex locateIndex(const CRelIndex &ndxRel) const;

	CVerseList verseList() const;
	void setVerseList(const CVerseList &verses);

	TParsedPhrasesList parsedPhrases() const;
	TPhraseTagList setParsedPhrases(const CSearchCriteria &aSearchCriteria, const TParsedPhrasesList &phrases);		// Will build verseList and return the list of tags so they can be passed to a highlighter, etc

	VERSE_DISPLAY_MODE_ENUM displayMode() const { return m_nDisplayMode; }
	VERSE_TREE_MODE_ENUM treeMode() const { return m_nTreeMode; }
	bool showMissingLeafs() const { return m_bShowMissingLeafs; }

	int GetResultsCount(unsigned int nBk = 0, unsigned int nChp = 0) const;				// Calculates the total number of results from the Parsed Phrases (can be limited to book or book/chapter)

	QPair<int, int> GetResultsIndexes(int nVerse) const;	// Calculates the starting and ending results indexes for the specified Verse List entry index
	QPair<int, int> GetBookIndexAndCount(int nVerse = -1) const;	// Returns the Search Result Book number and total number of books with results
	QPair<int, int> GetChapterIndexAndCount(int nVerse = -1) const;	// Returns the Search Result Chapter and total number of chapters with results
	QPair<int, int> GetVerseIndexAndCount(int nVerse = -1) const;	// Returns the Search Result Verse and total number of verses with results (for completeness only)

	bool hasExceededDisplayLimit() const;

signals:

public slots:
	void setDisplayMode(CVerseListModel::VERSE_DISPLAY_MODE_ENUM nDisplayMode);
	void setTreeMode(CVerseListModel::VERSE_TREE_MODE_ENUM nTreeMode);
	void setShowMissingLeafs(bool bShowMissing);
	virtual void setFont(const QFont& aFont);

protected:
	int GetBookCount() const;						// Returns the number of books in the model based on mode
	int IndexByBook(unsigned int nBk) const;		// Returns the index (in the number of books) for the specified Book number
	unsigned int BookByIndex(int ndxBook) const;	// Returns the Book Number for the specified index (in the number of books)
	int GetChapterCount(unsigned int nBk) const;	// Returns the number of chapters in the specified book number based on the current mode
	int IndexByChapter(unsigned int nBk, unsigned int nChp) const;	// Returns the index (in the number of chapters) for the specified Chapter number
	unsigned int ChapterByIndex(int ndxBook, int ndxChapter) const;		// Returns the Chapter Number for the specified index (in the number of chapters)
	int FindVerseIndex(const CRelIndex &ndxRel) const;	// Looks for the specified CRelIndex in the m_lstVerses array and returns its index
public:
	int GetVerseCount(unsigned int nBk = 0, unsigned int nChp = 0) const;
	int GetVerse(int ndxVerse, unsigned int nBk, unsigned int nChp = 0) const;	// Returns index into m_lstVerses based on relative index of Verse for specified Book and/or Book/Chapter

private:
	void buildScopedResultsInParsedPhrases();
	TPhraseTagList buildVerseListFromParsedPhrases();
	CRelIndex ScopeIndex(const CRelIndex &index, CSearchCriteria::SEARCH_SCOPE_MODE_ENUM nMode);

private:
	Q_DISABLE_COPY(CVerseListModel)
	CBibleDatabasePtr m_pBibleDatabase;
	CVerseList m_lstVerses;
	QMap<uint32_t, int> m_mapVerses;			// Reverse lookup for verses for tree.  Map of CRelIndex->index() to index within m_lstVerses.  Set during setVerseList.
	QMap<uint32_t, QSize> m_mapSizeHints;		// Map of CRelIndex->index() to SizeHint -- used for ReflowDelegate caching (Note: This only needs to be cleared if we change databases or display modes!)
	TParsedPhrasesList m_lstParsedPhrases;		// Parsed phrases, updated by KJVCanOpener on_phraseChanged
	CSearchCriteria m_SearchCriteria;			// Search criteria set during setParsedPhrases
	VERSE_DISPLAY_MODE_ENUM m_nDisplayMode;
	VERSE_TREE_MODE_ENUM m_nTreeMode;
	bool m_bShowMissingLeafs;					// Shows the missing leafs in book or book/chapter modes
};

// ============================================================================

#endif // VERSELISTMODEL_H
