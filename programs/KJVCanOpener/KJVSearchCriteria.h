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

#ifndef KJVSEARCHCRITERIA_H
#define KJVSEARCHCRITERIA_H

#include "dbstruct.h"

#include <QString>
#include <QStringList>
#include <QObject>
#include <QWidget>
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QList>
#include <QMap>

// ============================================================================

// Forward Declarations:
class CSearchWithinModel;

// ============================================================================

// Set these to the minimum and maximum Search Scope Modes (used for limits bounds checking)
#define SSME_MINIMUM CSearchCriteria::SSME_WHOLE_BIBLE
#define SSME_MAXIMUM CSearchCriteria::SSME_CATEGORY

class CSearchCriteria
{
public:
	enum SEARCH_SCOPE_MODE_ENUM {
		SSME_WHOLE_BIBLE = 0,
		SSME_TESTAMENT = 1,
		SSME_BOOK = 2,
		SSME_CHAPTER = 3,
		SSME_VERSE = 4,
		SSME_CATEGORY = 5
	};

	CSearchCriteria()
		:	m_nSearchScopeMode(SSME_WHOLE_BIBLE) { }

	virtual ~CSearchCriteria() { }

	SEARCH_SCOPE_MODE_ENUM searchScopeMode() const { return m_nSearchScopeMode; }
	void setSearchScopeMode(SEARCH_SCOPE_MODE_ENUM nMode) { m_nSearchScopeMode = nMode; }

	bool indexIsWithin(const CRelIndex &ndxRel) const
	{
		return (m_setSearchWithin.find(CRelIndex(ndxRel.book(), 0, 0, 0)) != m_setSearchWithin.end());
	}
	bool withinIsEntireBible(CBibleDatabasePtr pBibleDatabase) const
	{
		assert(pBibleDatabase != NULL);
		bool bIsEntire = true;
		for (uint32_t nBk = 1; ((bIsEntire) && (nBk <= pBibleDatabase->bibleEntry().m_nNumBk)); ++nBk) {
			if (m_setSearchWithin.find(CRelIndex(nBk, 0, 0, 0)) == m_setSearchWithin.end()) bIsEntire = false;
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
			assert(pBibleDatabase != NULL);
			for (uint32_t nBk = 1; nBk <= pBibleDatabase->bibleEntry().m_nNumBk; ++nBk) {
				m_setSearchWithin.insert(CRelIndex(nBk, 0, 0, 0));
			}
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
	QString searchScopeDescription() const;

private:
	SEARCH_SCOPE_MODE_ENUM m_nSearchScopeMode;
	TRelativeIndexSet m_setSearchWithin;
};

// ============================================================================

namespace Ui {
class CKJVSearchCriteriaWidget;
}

class CKJVSearchCriteriaWidget : public QWidget
{
	Q_OBJECT

public:
	explicit CKJVSearchCriteriaWidget(QWidget *parent = 0);
	~CKJVSearchCriteriaWidget();

	void initialize(CBibleDatabasePtr pBibleDatabase);

	const CSearchCriteria &searchCriteria() const { return m_SearchCriteria; }

signals:
	void changedSearchCriteria();
	void addSearchPhraseClicked();
	void copySearchPhraseSummary();

public slots:
	void enableCopySearchPhraseSummary(bool bEnable);
	void setSearchScopeMode(CSearchCriteria::SEARCH_SCOPE_MODE_ENUM mode);

	void setSearchWithin(const TRelativeIndexSet &aSetSearchWithin);
	void setSearchWithin(const QString &strSearchWithin);

	void setTextBrightness(bool bInvert, int nBrightness);
	void setAdjustDialogElementBrightness(bool bAdjust);

private slots:
	void en_changedSearchScopeMode(int ndx);
	void en_changedSearchWithin();

// Data Private:
private:
	CBibleDatabasePtr m_pBibleDatabase;
	CSearchCriteria m_SearchCriteria;
	CSearchWithinModel *m_pSearchWithinModel;

// UI Private:
private:
	bool m_bDoingUpdate;		// True if combo boxes, etc, are being updated and change notifications should be ignored

#define begin_update()							\
			CBusyCursor iAmBusy(NULL);			\
			bool bUpdateSave = m_bDoingUpdate;	\
			m_bDoingUpdate = true;
#define end_update()							\
			m_bDoingUpdate = bUpdateSave;

	Ui::CKJVSearchCriteriaWidget *ui;
};

// ============================================================================

class CSearchWithinModelIndex : public QObject
{
	Q_OBJECT

public:
	CSearchWithinModelIndex(CSearchCriteria::SEARCH_SCOPE_MODE_ENUM assme = CSearchCriteria::SSME_WHOLE_BIBLE, uint32_t nItemIndex = 0, CSearchWithinModelIndex *pParentIndex = NULL)
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
	int indexOfChild(const CSearchWithinModelIndex *pChild) const { return m_lstChildren.indexOf(pChild); }
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
	uint32_t m_nItemIndex;								// Book, Testament, Category, etc, index
	QList<const CSearchWithinModelIndex *> m_lstChildren;		// Collection of objects that are children to this object
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
	CSearchWithinModel(CBibleDatabasePtr pBibleDatabase, const TRelativeIndexSet &aSetSearchWithin, QObject *pParent = 0);
	virtual ~CSearchWithinModel();

	QString searchWithinDescription() const;
	TRelativeIndexSet searchWithin() const;
	void setSearchWithin(const TRelativeIndexSet &aSetSearchWithin);

	virtual int rowCount(const QModelIndex &zParent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex &zParent = QModelIndex()) const;

	virtual QModelIndex	index(int row, int column = 0, const QModelIndex &zParent = QModelIndex()) const;
	virtual QModelIndex parent(const QModelIndex &index) const;

	virtual QVariant data(const QModelIndex &index, int role) const;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

	virtual Qt::ItemFlags flags(const QModelIndex &index) const;

	virtual bool insertRows(int row, int count, const QModelIndex &zParent = QModelIndex());
	virtual bool removeRows(int row, int count, const QModelIndex &zParent = QModelIndex());

	virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

	static const CSearchWithinModelIndex *toSearchWithinModelIndex(const QModelIndex &ndx) {
		return reinterpret_cast<const CSearchWithinModelIndex *>(ndx.internalPointer());
	}
	static void *fromSearchWithinModelIndex(const CSearchWithinModelIndex *pIndex) { return reinterpret_cast<void *>(const_cast<CSearchWithinModelIndex *>(pIndex)); }

signals:
	void changedSearchWithin();

private:
	void fireChildrenChange(const QModelIndex &index);

// Data Private:
private:
	Q_DISABLE_COPY(CSearchWithinModel)
	CBibleDatabasePtr m_pBibleDatabase;
	CSearchWithinModelIndex m_rootSearchWithinModelIndex;
};

// ============================================================================

#endif // KJVSEARCHCRITERIA_H
