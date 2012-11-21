#ifndef VERSELISTMODEL_H
#define VERSELISTMODEL_H

#include "dbstruct.h"

#include <QAbstractListModel>
#include <QModelIndex>
#include <QList>
#include <QStringList>
#include <QVariant>

// ============================================================================

class CVerseListItem
{
public:
	explicit CVerseListItem(const CRelIndex &ndx = CRelIndex(),
							const QString &strHeading = QString(),
							const QString &strToolTip = QString())
		:	m_ndxRelative(ndx),
			m_strHeading(strHeading),
			m_strToolTip(strToolTip)
	{ }
	~CVerseListItem()
	{ }

	QString getHeading() const { return m_strHeading; }
	void setHeading(const QString &strHeading) { m_strHeading = strHeading; }

	QString getToolTip() const { return m_strToolTip; }
	void setToolTip(const QString &strToolTip) { m_strToolTip = strToolTip; }

	bool isSet() const {
		return (m_ndxRelative.isSet());
	}

	uint32_t getBook() const { return m_ndxRelative.book(); }			// Book Number (1-n)
	uint32_t getChapter() const { return m_ndxRelative.chapter(); }		// Chapter Number within Book (1-n)
	uint32_t getVerse() const { return m_ndxRelative.verse(); }			// Verse Number within Chapter (1-n)
	uint32_t getWord() const { return m_ndxRelative.word(); }			// Word Number within Verse (1-n)
	void setIndexNormalized(uint32_t ndx) { m_ndxRelative = CRelIndex(DenormalizeIndex(ndx)); }
	void setIndexDenormalized(uint32_t ndx) { m_ndxRelative = CRelIndex(ndx); }
	uint32_t getIndexNormalized() const { return NormalizeIndex(m_ndxRelative.index()); }
	uint32_t getIndexDenormalized() const { return m_ndxRelative.index(); }
	CRelIndex getIndex() const { return m_ndxRelative; }
	TPhraseTagList &phraseTags() { return m_lstTags; }
	const TPhraseTagList &phraseTags() const { return m_lstTags; }

	QStringList getWordList() const
	{
		if (!isSet()) return QStringList();
		QStringList strWords;
		unsigned int nNumWords = (g_lstBooks[getBook()-1])[CRelIndex(0,getChapter(),getVerse(),0)].m_nNumWrd;
		uint32_t ndxNormal = NormalizeIndex(m_ndxRelative.index());
		while (nNumWords) {
			strWords.push_back(g_lstConcordanceWords[g_lstConcordanceMapping[ndxNormal]-1]);
			ndxNormal++;
			nNumWords--;
		}
		return strWords;
	}
	QString getVerseVeryPlainText() const		// Very Plain has no punctuation!
	{
		return getWordList().join(" ");
	}
	QString getVerseRichText() const
	{
		if (!isSet()) return QString();
		return (g_lstBooks[m_ndxRelative.book()-1])[CRelIndex(0,m_ndxRelative.chapter(),m_ndxRelative.verse(),0)].GetRichText();
	}

private:
	CRelIndex m_ndxRelative;		// Relative Index
	TPhraseTagList m_lstTags;		// Phrase Tags to highlight
	QString m_strHeading;
	QString m_strToolTip;
};

Q_DECLARE_METATYPE(CVerseListItem)

typedef QList<CVerseListItem> CVerseList;

// ============================================================================

class CVerseListModel : public QAbstractListModel
{
	Q_OBJECT
public:
	enum VERSE_DISPLAY_MODE_ENUM {
		VDME_HEADING = 0,
		VDME_VERYPLAIN = 1,
		VDME_RICHTEXT = 2,
		VDME_COMPLETE = 3
	};

	enum VERSE_DATA_ROLES_ENUM {
		VERSE_ENTRY_ROLE = Qt::UserRole + 0
	};

	explicit CVerseListModel(QObject *parent = 0);
	CVerseListModel(const CVerseList &verses, VERSE_DISPLAY_MODE_ENUM nDisplayMode = VDME_HEADING, QObject *parent = 0);

	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;

	virtual QVariant data(const QModelIndex &index, int role) const;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

	virtual Qt::ItemFlags flags(const QModelIndex &index) const;

	virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
	virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

	virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

	virtual Qt::DropActions supportedDropActions() const;

	CVerseList verseList() const;
	void setVerseList(const CVerseList &verses);

	VERSE_DISPLAY_MODE_ENUM displayMode() const { return m_nDisplayMode; }
	void setDisplayMode(VERSE_DISPLAY_MODE_ENUM nDisplayMode) { m_nDisplayMode = nDisplayMode; }

signals:

public slots:

private:
	Q_DISABLE_COPY(CVerseListModel)
	CVerseList m_lstVerses;
	VERSE_DISPLAY_MODE_ENUM m_nDisplayMode;
};

#endif // VERSELISTMODEL_H
