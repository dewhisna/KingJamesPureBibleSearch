#ifndef VERSELISTMODEL_H
#define VERSELISTMODEL_H

#include "dbstruct.h"
#include "PhraseEdit.h"
#include "KJVSearchCriteria.h"

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QList>
#include <QMap>
#include <QStringList>
#include <QVariant>
#include <QPair>

#include <assert.h>

// ============================================================================

class CVerseListItem
{
public:
	explicit CVerseListItem(const CRelIndex &ndx = CRelIndex(),
							const unsigned int nPhraseSize = 1)
		:	m_ndxRelative(ndx)
	{
		m_ndxRelative.setWord(0);								// Primary index will have a zero word index
		if (nPhraseSize > 0)
			m_lstTags.push_back(TPhraseTag(ndx, nPhraseSize));		// But the corresponding tag will have non-zero word index
	}
	CVerseListItem(const TPhraseTag &tag)
		:	m_ndxRelative(tag.first)
	{
		m_ndxRelative.setWord(0);				// Primary index will have a zero word index
		if (tag.second > 0)
			m_lstTags.push_back(tag);			// But the corresponding tag will have non-zero word index
	}
	~CVerseListItem()
	{ }

	inline QString getHeading() const {
		QString strHeading;
		if (m_lstTags.size() > 0) strHeading += QString("(%1) ").arg(m_lstTags.size());
		for (int ndx = 0; ndx < m_lstTags.size(); ++ndx) {
			if (ndx == 0) {
				strHeading += m_lstTags.at(ndx).first.PassageReferenceText();
			} else {
				strHeading += QString("[%1]").arg(m_lstTags.at(ndx).first.word());
			}
		}
		return strHeading;
	}

	inline QString getToolTip(const TParsedPhrasesList &phrases = TParsedPhrasesList()) const {
		QString strToolTip;
		strToolTip += getIndex().SearchResultToolTip(RIMASK_BOOK | RIMASK_CHAPTER | RIMASK_VERSE);
		for (int ndx = 0; ndx < phraseTags().size(); ++ndx) {
			const CRelIndex &ndxTag(phraseTags().at(ndx).first);
			if (phraseTags().size() > 1) {
				strToolTip += QString("(%1)[%2] \"%3\" is ").arg(ndx+1).arg(ndxTag.word()).arg(getPhrase(ndx));
			} else {
				strToolTip += QString("[%1] \"%2\" is ").arg(ndxTag.word()).arg(getPhrase(ndx));
			}
			strToolTip += ndxTag.SearchResultToolTip(RIMASK_WORD);
			for (int ndxPhrase = 0; ndxPhrase < phrases.size(); ++ndxPhrase) {
				const CParsedPhrase *pPhrase = phrases.at(ndxPhrase);
				assert(pPhrase != NULL);
				if (pPhrase == NULL) continue;
				if (pPhrase->GetPhraseTagSearchResults().contains(phraseTags().at(ndx))) {
					strToolTip += QString("    %1 of %2 of Search Phrase \"%3\" Results in Entire Bible\n")
										.arg(pPhrase->GetPhraseTagSearchResults().indexOf(phraseTags().at(ndx)) + 1)
										.arg(pPhrase->GetPhraseTagSearchResults().size())
										.arg(pPhrase->phrase());
				}
				if (pPhrase->GetScopedPhraseTagSearchResults().contains(phraseTags().at(ndx))) {
					strToolTip += QString("    %1 of %2 of Search Phrase \"%3\" Results in Search Scope\n")
										.arg(pPhrase->GetScopedPhraseTagSearchResults().indexOf(phraseTags().at(ndx)) + 1)
										.arg(pPhrase->GetScopedPhraseTagSearchResults().size())
										.arg(pPhrase->phrase());
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
	uint32_t getIndexNormalized() const { return NormalizeIndex(m_ndxRelative.index()); }
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
		assert((nTag >= 0) && (nTag < m_lstTags.size()));
		if ((!isSet()) || (nTag < 0) || (nTag >= m_lstTags.size())) return QStringList();
		QStringList strWords;
		unsigned int nNumWords = m_lstTags.at(nTag).second;
		uint32_t ndxNormal = NormalizeIndex(m_lstTags.at(nTag).first);
		while (nNumWords) {
			strWords.push_back(g_lstConcordanceWords[g_lstConcordanceMapping[ndxNormal]-1]);
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
		if (!isSet()) return QStringList();
		QStringList strWords;
		unsigned int nNumWords = (g_lstBooks[getBook()-1])[CRelIndex(0,getChapter(),getVerse(),0)].m_nNumWrd;
		uint32_t ndxNormal = getIndexNormalized();
		while (nNumWords) {
			strWords.push_back(g_lstConcordanceWords[g_lstConcordanceMapping[ndxNormal]-1]);
			ndxNormal++;
			nNumWords--;
		}
		return strWords;
	}
	QString getVerseVeryPlainText() const		// Very Plain has no punctuation!
	{
		return getVerseAsWordList().join(" ");
	}
	QString getVerseRichText() const
	{
		if (!isSet()) return QString();
		return (g_lstBooks[getBook()-1])[CRelIndex(0,getChapter(),getVerse(),0)].GetRichText();
	}

private:
	CRelIndex m_ndxRelative;		// Primary Relative Index (word index == 0)
	TPhraseTagList m_lstTags;		// Phrase Tags to highlight, includes a copy of the Primary (w/word index != 0)
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
		VERSE_ENTRY_ROLE = Qt::UserRole + 0,
		TOOLTIP_PLAINTEXT_ROLE = Qt::UserRole + 1,			// Same as Qt::ToolTipRole, but as PlainText instead of RichText
		TOOLTIP_NOHEADING_ROLE = Qt::UserRole + 2,			// Same as Qt::ToolTipRole, but without Verse Reference Heading
		TOOLTIP_NOHEADING_PLAINTEXT_ROLE = Qt::UserRole + 3	// Same as TOOLTIP_PLAINTEXT_ROLE, but without Verse Reference Heading
	};

	explicit CVerseListModel(QObject *parent = 0);
	CVerseListModel(const CVerseList &verses, QObject *parent = 0);

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

	CVerseList verseList() const;
	void setVerseList(const CVerseList &verses);

	TParsedPhrasesList parsedPhrases() const;
	TPhraseTagList setParsedPhrases(CKJVSearchCriteria::SEARCH_SCOPE_MODE_ENUM nSearchScopeMode, const TParsedPhrasesList &phrases);		// Will build verseList and return the list of tags so they can be passed to a highlighter, etc

	VERSE_DISPLAY_MODE_ENUM displayMode() const { return m_nDisplayMode; }
	void setDisplayMode(VERSE_DISPLAY_MODE_ENUM nDisplayMode);

	VERSE_TREE_MODE_ENUM treeMode() const { return m_nTreeMode; }
	void setTreeMode(VERSE_TREE_MODE_ENUM nTreeMode);

	bool showMissingLeafs() const { return m_bShowMissingLeafs; }
	void setShowMissingLeafs(bool bShowMissing);

	int GetResultsCount(unsigned int nBk = 0, unsigned int nChp = 0) const;				// Calculates the total number of results from the Parsed Phrases (can be limited to book or book/chapter)

	QPair<int, int> GetResultsIndexes(int nVerse) const;	// Calculates the starting and ending results indexes for the specified Verse List entry index
	QPair<int, int> GetBookIndexAndCount(int nVerse = -1) const;	// Returns the Search Result Book number and total number of books with results
	QPair<int, int> GetChapterIndexAndCount(int nVerse = -1) const;	// Returns the Search Result Chapter and total number of chapters with results
	QPair<int, int> GetVerseIndexAndCount(int nVerse = -1) const;	// Returns the Search Result Verse and total number of verses with results (for completeness only)

	bool hasExceededDisplayLimit() const;

signals:

public slots:

protected:
	int GetBookCount() const;						// Returns the number of books in the model based on mode
	int IndexByBook(unsigned int nBk) const;		// Returns the index (in the number of books) for the specified Book number
	unsigned int BookByIndex(int ndxBook) const;	// Returns the Book Number for the specified index (in the number of books)
	int GetChapterCount(unsigned int nBk) const;	// Returns the number of chapters in the specified book number based on the current mode
	int IndexByChapter(unsigned int nBk, unsigned int nChp) const;	// Returns the index (in the number of chapters) for the specified Chapter number
	unsigned int ChapterByIndex(int ndxBook, int ndxChapter) const;		// Returns the Chapter Number for the specified index (in the number of chapters)
public:
	int GetVerseCount(unsigned int nBk, unsigned int nChp = 0) const;
	int GetVerse(int ndxVerse, unsigned int nBk, unsigned int nChp = 0) const;	// Returns index into m_lstVerses based on relative index of Verse for specified Book and/or Book/Chapter

private:
	void buildScopedResultsInParsedPhrases();
	TPhraseTagList buildVerseListFromParsedPhrases();
	static CRelIndex ScopeIndex(const CRelIndex &index, CKJVSearchCriteria::SEARCH_SCOPE_MODE_ENUM nMode);

private:
	Q_DISABLE_COPY(CVerseListModel)
	CVerseList m_lstVerses;
	QMap<uint32_t, int> m_mapVerses;			// Reverse lookup for verses for tree.  Map of CRelIndex->index() to index within m_lstVerses.  Set during setVerseList.
	TParsedPhrasesList m_lstParsedPhrases;		// Parsed phrases, updated by KJVCanOpener on_phraseChanged
	CKJVSearchCriteria::SEARCH_SCOPE_MODE_ENUM m_nSearchScopeMode;	// Last search scope set during setParsedPhrases
	VERSE_DISPLAY_MODE_ENUM m_nDisplayMode;
	VERSE_TREE_MODE_ENUM m_nTreeMode;
	bool m_bShowMissingLeafs;					// Shows the missing leafs in book or book/chapter modes
};

// ============================================================================

#endif // VERSELISTMODEL_H
