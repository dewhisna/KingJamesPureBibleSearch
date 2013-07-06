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

#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include "dbstruct.h"

#include <QObject>
#include <QTextCharFormat>

// ============================================================================

// Forward Declarations:
class CVerseListModel;
class CVerseListItem;
typedef QMap<CRelIndex, CVerseListItem> CVerseMap;				// Redundant with definition in VerseListModel.h, but avoids very nasty header interdependency

// ============================================================================

class CHighlighterPhraseTagFwdItr
{
protected:
	CHighlighterPhraseTagFwdItr(const CVerseListModel *pVerseListModel);
	CHighlighterPhraseTagFwdItr(const TPhraseTagList &lstTags);

public:
	TPhraseTag nextTag();
	bool isEnd() const;

private:
	const CVerseListModel *m_pVerseListModel;
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
	explicit CBasicHighlighter(QObject *parent = NULL)
		:	QObject(parent),
			m_bEnabled(true)
	{
	}

	virtual void doHighlighting(QTextCharFormat &aFormat, bool bClear) const = 0;
	virtual bool enabled() const { return m_bEnabled; }

	virtual CHighlighterPhraseTagFwdItr getForwardIterator() const = 0;
	virtual bool isEmpty() const = 0;

	virtual bool isContinuous() const { return false; }			// Continuous = no word breaks in highlighting.  Default is false

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
	explicit CSearchResultHighlighter(CVerseListModel *pVerseListModel, QObject *parent = NULL);
	CSearchResultHighlighter(const TPhraseTagList &lstPhraseTags, QObject *parent = NULL);
	CSearchResultHighlighter(const TPhraseTag &aTag, QObject *parent = NULL);
	virtual ~CSearchResultHighlighter();

	virtual void doHighlighting(QTextCharFormat &aFormat, bool bClear) const;

	virtual CHighlighterPhraseTagFwdItr getForwardIterator() const;
	virtual bool isEmpty() const;

private slots:
	void verseListChanged();
	void verseListModelDestroyed();

private:
	CVerseListModel *m_pVerseListModel;

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
	explicit CCursorFollowHighlighter(const TPhraseTagList &lstPhraseTags = TPhraseTagList(), QObject *parent = NULL)
		:	CBasicHighlighter(parent)
	{
		m_myPhraseTags.setPhraseTags(lstPhraseTags);
	}
	CCursorFollowHighlighter(const TPhraseTag &aTag, QObject *parent = NULL)
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

	virtual void doHighlighting(QTextCharFormat &aFormat, bool bClear) const;

	virtual CHighlighterPhraseTagFwdItr getForwardIterator() const;
	virtual bool isEmpty() const;

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
	explicit CUserDefinedHighlighter(const QString &strUserDefinedHighlighterName, const TPhraseTagList &lstPhraseTags = TPhraseTagList(), QObject *parent = NULL)
		:	CBasicHighlighter(parent),
			m_strUserDefinedHighlighterName(strUserDefinedHighlighterName)
	{
		m_myPhraseTags.setPhraseTags(lstPhraseTags);
	}
	CUserDefinedHighlighter(const QString &strUserDefinedHighlighterName, const TPhraseTag &aTag, QObject *parent = NULL)
		:	CBasicHighlighter(parent),
			m_strUserDefinedHighlighterName(strUserDefinedHighlighterName)
	{
		TPhraseTagList lstTags;
		lstTags.append(aTag);
		m_myPhraseTags.setPhraseTags(lstTags);
	}
	CUserDefinedHighlighter(const CUserDefinedHighlighter &aUserDefinedHighlighter)
		:	CBasicHighlighter(aUserDefinedHighlighter.parent())
	{
		setEnabled(aUserDefinedHighlighter.enabled());
		m_myPhraseTags.setPhraseTags(aUserDefinedHighlighter.m_myPhraseTags.phraseTags());
		m_strUserDefinedHighlighterName = aUserDefinedHighlighter.m_strUserDefinedHighlighterName;
	}

	virtual void doHighlighting(QTextCharFormat &aFormat, bool bClear) const;

	virtual CHighlighterPhraseTagFwdItr getForwardIterator() const;
	virtual bool isEmpty() const;

	virtual bool isContinuous() const { return true; }

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

#endif // HIGHLIGHTER_H
