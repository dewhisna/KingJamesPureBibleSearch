/****************************************************************************
**
** Copyright (C) 2012-2020 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef SEARCH_CRITERIA_H
#define SEARCH_CRITERIA_H

#include "dbstruct.h"

#include <QString>
#include <QStringList>
#include <QObject>
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QModelIndexList>
#include <QList>
#include <QMap>

// ============================================================================

// Set these to the minimum and maximum Search Scope Modes (used for limits bounds checking)
#define SSME_MINIMUM CSearchCriteria::SSME_UNSCOPED
#define SSME_MAXIMUM CSearchCriteria::SSME_VERSE

class CSearchCriteria
{
public:
	// Note: These values can't be changed without breaking KJS files! (not to mention
	//			persistent settings)
	enum SEARCH_SCOPE_MODE_ENUM {
		SSME_UNSCOPED = -1,
		SSME_WHOLE_BIBLE = 0,
		SSME_TESTAMENT = 1,
		SSME_BOOK = 2,
		SSME_CHAPTER = 3,
		SSME_VERSE = 4,
		SSME_CATEGORY = 5,					// Scoping to Category is now deprecated and will devolve to SSME_TESTAMENT
		SSME_COLOPHON = 6,
		SSME_SUPERSCRIPTION = 7
	};

	// Special Search Indexes:
	static const CRelIndex SSI_COLOPHON;
	static const CRelIndex SSI_SUPERSCRIPTION;

	CSearchCriteria()
		:	m_nSearchScopeMode(SSME_UNSCOPED)
	{ }

	virtual ~CSearchCriteria() { }

	void clear()
	{
		m_nSearchScopeMode = SSME_UNSCOPED;
		m_setSearchWithin.clear();
	}

	SEARCH_SCOPE_MODE_ENUM searchScopeMode() const { return m_nSearchScopeMode; }
	void setSearchScopeMode(SEARCH_SCOPE_MODE_ENUM nMode) { m_nSearchScopeMode = nMode; }

	bool indexIsWithin(const CRelIndex &ndxRel) const
	{
		if ((ndxRel.book() != 0) &&
			(ndxRel.verse() == 0) &&
			(ndxRel.word() != 0)) {
			if (ndxRel.chapter() == 0) {
				return ((m_setSearchWithin.find(SSI_COLOPHON) != m_setSearchWithin.end()) &&
						((m_setSearchWithin.find(CRelIndex(ndxRel.book(), 0, 0, 0)) != m_setSearchWithin.end()) || withinIsPseudoVerseOnly()));
			} else {
				return ((m_setSearchWithin.find(SSI_SUPERSCRIPTION) != m_setSearchWithin.end()) &&
						((m_setSearchWithin.find(CRelIndex(ndxRel.book(), 0, 0, 0)) != m_setSearchWithin.end()) || withinIsPseudoVerseOnly()));
			}
		}
		return (m_setSearchWithin.find(CRelIndex(ndxRel.book(), 0, 0, 0)) != m_setSearchWithin.end());
	}
	bool phraseIsCompletelyWithin(CBibleDatabasePtr pBibleDatabase, const TPhraseTag &phraseTag) const
	{
		Q_ASSERT(!pBibleDatabase.isNull());
		TTagBoundsPair phraseBounds(phraseTag.bounds(pBibleDatabase.data()));
		CRelIndex ndxLo = pBibleDatabase->DenormalizeIndex(phraseBounds.lo());
		CRelIndex ndxHi = pBibleDatabase->DenormalizeIndex(phraseBounds.hi());
		bool bIsWithin = indexIsWithin(ndxLo) && indexIsWithin(ndxHi);		// This to handle Superscriptions and Colophons
		for (uint32_t nBk = ndxLo.book(); ((nBk <= ndxHi.book()) && (bIsWithin)); ++nBk) {
			if (!indexIsWithin(CRelIndex(nBk, 0, 0, 0))) bIsWithin = false;
		}
		return bIsWithin;
	}
	bool withinIsPseudoVerseOnly() const
	{
		bool bColophon = (m_setSearchWithin.find(SSI_COLOPHON) != m_setSearchWithin.end());
		bool bSuperscription = (m_setSearchWithin.find(SSI_SUPERSCRIPTION) != m_setSearchWithin.end());
		return (((bColophon) && (!bSuperscription) && (m_setSearchWithin.size() == 1)) ||
				((!bColophon) && (bSuperscription) && (m_setSearchWithin.size() == 1)) ||
				((bColophon) && (bSuperscription) && (m_setSearchWithin.size() == 2)));
	}
	bool withinIsEntireBible(CBibleDatabasePtr pBibleDatabase, bool bIgnorePseudoVerses = false) const
	{
		Q_ASSERT(!pBibleDatabase.isNull());
		bool bIsEntire = true;
		for (uint32_t nBk = 1; ((bIsEntire) && (nBk <= pBibleDatabase->bibleEntry().m_nNumBk)); ++nBk) {
			if (m_setSearchWithin.find(CRelIndex(nBk, 0, 0, 0)) == m_setSearchWithin.end()) bIsEntire = false;
		}
		if (!bIgnorePseudoVerses) {
			if ((pBibleDatabase->hasColophons()) && (m_setSearchWithin.find(SSI_COLOPHON) == m_setSearchWithin.end())) bIsEntire = false;
			if ((pBibleDatabase->hasSuperscriptions()) && (m_setSearchWithin.find(SSI_SUPERSCRIPTION) == m_setSearchWithin.end())) bIsEntire = false;
		}
		return bIsEntire;
	}
	const TRelativeIndexSet &searchWithin() const { return m_setSearchWithin; }
	void setSearchWithin(const TRelativeIndexSet &aSetSearchWithin) { m_setSearchWithin = aSetSearchWithin; }
	void setSearchWithin(CBibleDatabasePtr pBibleDatabase, const QString &strSearchWithin = QString())
	{
		QStringList lstIndexes = strSearchWithin.split(QChar(','));
		m_setSearchWithin.clear();
		if (strSearchWithin.compare("-") == 0) return;
		// Check for "Entire Bible" shortcut (i.e. empty string or single "0"):
		if ((lstIndexes.size() == 0) ||
			((lstIndexes.size() == 1) && (lstIndexes.at(0).toUInt() == 0))) {
			Q_ASSERT(!pBibleDatabase.isNull());
			for (uint32_t nBk = 1; nBk <= pBibleDatabase->bibleEntry().m_nNumBk; ++nBk) {
				m_setSearchWithin.insert(CRelIndex(nBk, 0, 0, 0));
			}
			m_setSearchWithin.insert(SSI_COLOPHON);
			m_setSearchWithin.insert(SSI_SUPERSCRIPTION);
		} else {
			for (int ndx = 0; ndx < lstIndexes.size(); ++ndx) {
				CRelIndex ndxRel(lstIndexes.at(ndx));
				if (ndxRel.isSet()) m_setSearchWithin.insert(ndxRel);
			}
		}
	}
	QString searchWithinToString() const
	{
		if (m_setSearchWithin.size() == 0) return QString("-");
		QStringList lstIndexes;
		for (TRelativeIndexSet::const_iterator itrIndexes = m_setSearchWithin.begin(); itrIndexes != m_setSearchWithin.end(); ++itrIndexes) {
			lstIndexes.append(itrIndexes->asAnchor());
		}
		return lstIndexes.join(QString(","));
	}
	QString searchWithinDescription(CBibleDatabasePtr pBibleDatabase) const;
	static QString searchScopeDescription(SEARCH_SCOPE_MODE_ENUM ssmeScope);
	QString searchScopeDescription() const;

private:
	SEARCH_SCOPE_MODE_ENUM m_nSearchScopeMode;
	TRelativeIndexSet m_setSearchWithin;
};

// ============================================================================

class CSearchWithinModelIndex : public QObject
{
	Q_OBJECT

public:
	CSearchWithinModelIndex(CSearchCriteria::SEARCH_SCOPE_MODE_ENUM assme = CSearchCriteria::SSME_WHOLE_BIBLE, uint32_t nItemIndex = 0, CSearchWithinModelIndex *pParentIndex = nullptr)
		:	QObject(pParentIndex),
			m_ssme(assme),
			m_nItemIndex(nItemIndex),
			m_pParentIndex(pParentIndex),
			m_nLevel(0),
			m_bChecked(true)
	{
		const CSearchWithinModelIndex *pPrevious = parentIndex();
		while (pPrevious) {
			m_nLevel++;
			pPrevious = pPrevious->parentIndex();
		}
	}
	~CSearchWithinModelIndex() { }

	void clear()
	{
		for (int ndxChild = 0; ndxChild < m_lstChildren.size(); ++ndxChild) {
			m_lstChildren[ndxChild]->clear();
		}
		m_lstChildren.clear();
		if (m_pParentIndex) deleteLater();
	}

	CSearchCriteria::SEARCH_SCOPE_MODE_ENUM ssme() const { return m_ssme; }
	uint32_t itemIndex() const { return m_nItemIndex; }
	const CSearchWithinModelIndex *parentIndex() const { return m_pParentIndex; }
	int level() const { return m_nLevel; }
	CSearchWithinModelIndex *insertIndex(CSearchCriteria::SEARCH_SCOPE_MODE_ENUM assme, uint32_t nItemIndex)
	{
		CSearchWithinModelIndex *pNewIndex = new CSearchWithinModelIndex(assme, nItemIndex, this);
		m_lstChildren.append(pNewIndex);
		return pNewIndex;
	}
	int childIndexCount() const { return m_lstChildren.size(); }
	const CSearchWithinModelIndex *childIndex(int ndx) const { return m_lstChildren.at(ndx); }
	int indexOfChild(const CSearchWithinModelIndex *pChild) const { return m_lstChildren.indexOf(const_cast<CSearchWithinModelIndex *>(pChild)); }		// const_cast required here for Qt4 Compat!
	Qt::CheckState checkState() const
	{
		if (m_lstChildren.size() == 0) return (m_bChecked ? Qt::Checked : Qt::Unchecked);
		Qt::CheckState nCheck = Qt::Unchecked;
		bool bOneIsUnchecked = false;
		for (int ndx = 0; ndx < m_lstChildren.size(); ++ndx) {
			switch (m_lstChildren.at(ndx)->checkState()) {
				case Qt::PartiallyChecked:
					return Qt::PartiallyChecked;								// If our child is partly checked, we are too
				case Qt::Checked:
					nCheck = Qt::Checked;
					if (bOneIsUnchecked) return Qt::PartiallyChecked;			// As soon as we hit one that is checked and already have one that isn't, we're done
					break;
				case Qt::Unchecked:
					bOneIsUnchecked = true;
					if (nCheck == Qt::Checked) return Qt::PartiallyChecked;		// As soon as we hit one that isn't checked and already have one that is, we're done
					break;
			}
		}
		return nCheck;
	}
	void setCheck(bool bCheck) const {
		if (m_lstChildren.size() == 0) {
			m_bChecked = bCheck;
		} else {
			for (int ndx = 0; ndx < m_lstChildren.size(); ++ndx) {
				m_lstChildren.at(ndx)->setCheck(bCheck);
			}
		}
	}

private:
	CSearchCriteria::SEARCH_SCOPE_MODE_ENUM m_ssme;		// Type of Object
	uint32_t m_nItemIndex;								// Book, Testament, Category, etc, index (For Category, this will be the BIBLE_BOOK_CATEGORIES_ENUM value for g_arrBibleBookCategories)
	QList<CSearchWithinModelIndex *> m_lstChildren;		// Collection of objects that are children to this object
	const CSearchWithinModelIndex *m_pParentIndex;
	int m_nLevel;										// Hierarchy level -- determined by ascending the parent nodes at creation
	mutable bool m_bChecked;							// Checkstate for this item

	Q_DISABLE_COPY(CSearchWithinModelIndex)
};

typedef QMap<uint32_t, CSearchWithinModelIndex *> CSearchWithinModelIndexMap;

class CSearchWithinModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	enum SEARCH_WITHIN_MODEL_DATA_ROLES_ENUM {
		SWMDRE_REL_INDEX_ROLE = Qt::UserRole + 0,		// Data role for CRelIndex() for the corresponding item relative index
		SWMDRE_WEBCHANNEL_ROLE = Qt::UserRole + 1		// Data role for toWebChannelJson() and toWebChannelHtml() format text
	};

	CSearchWithinModel(CBibleDatabasePtr pBibleDatabase, const CSearchCriteria &aSearchCriteria, QObject *pParent = nullptr);
	virtual ~CSearchWithinModel();

	QString searchWithinDescription() const;
	TRelativeIndexSet searchWithin() const;
	void setSearchWithin(const TRelativeIndexSet &aSetSearchWithin);

	virtual int rowCount(const QModelIndex &zParent = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex &zParent = QModelIndex()) const override;

	virtual QModelIndex	index(int row, int column = 0, const QModelIndex &zParent = QModelIndex()) const override;
	virtual QModelIndex parent(const QModelIndex &index) const override;

	virtual QVariant data(const QModelIndex &index, int role) const override;
	virtual QVariant data(const CSearchWithinModelIndex *pSearchWithinModelIndex, int role) const;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

	virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

	virtual bool insertRows(int row, int count, const QModelIndex &zParent = QModelIndex()) override;
	virtual bool removeRows(int row, int count, const QModelIndex &zParent = QModelIndex()) override;

	virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

	static const CSearchWithinModelIndex *toSearchWithinModelIndex(const QModelIndex &ndx) {
		return reinterpret_cast<const CSearchWithinModelIndex *>(ndx.internalPointer());
	}
	static void *fromSearchWithinModelIndex(const CSearchWithinModelIndex *pIndex) { return reinterpret_cast<void *>(const_cast<CSearchWithinModelIndex *>(pIndex)); }

	QModelIndexList getPersistentIndexList() const { return persistentIndexList(); }

#ifdef USING_WEBCHANNEL
	QString toWebChannelJson() const;
	QString toWebChannelHtml() const;
#endif

signals:
	void changedSearchWithin();

protected slots:
	void en_changedBibleDatabaseSettings(const QString &strUUID, const TBibleDatabaseSettings &aSettings);

private:
	void fireChildrenChange(const QModelIndex &index);
	void setupModel(const TRelativeIndexSet &aSetSearchWithin, bool bInitial = false);

// Data Private:
private:
	Q_DISABLE_COPY(CSearchWithinModel)
	CBibleDatabasePtr m_pBibleDatabase;
	CSearchWithinModelIndex m_rootSearchWithinModelIndex;
	bool m_bBibleHasColophons;
	bool m_bBibleHasSuperscriptions;
	BIBLE_BOOK_CATEGORY_GROUP_ENUM m_nLastCategoryGroup;	// Used to detect when the category has changed during Bible Database Option change
};

// ============================================================================

#endif // SEARCH_CRITERIA_H
