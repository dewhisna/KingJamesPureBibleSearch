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

class CBasicHighlighter : public QObject {
	Q_OBJECT
public:
	explicit CBasicHighlighter(const TPhraseTagList &lstPhraseTags = TPhraseTagList(), QObject *parent = NULL)
		:	QObject(parent),
			m_bEnabled(true)
	{
		m_myPhraseTags.setPhraseTags(lstPhraseTags);
	}
	CBasicHighlighter(const TPhraseTag &aTag, QObject *parent = NULL)
		:	QObject(parent),
			m_bEnabled(true)
	{
		TPhraseTagList lstTags;
		lstTags.append(aTag);
		m_myPhraseTags.setPhraseTags(lstTags);
	}
	CBasicHighlighter(const CBasicHighlighter &aHighlighter)
		:	QObject(aHighlighter.parent()),
			m_bEnabled(aHighlighter.m_bEnabled)
	{
		m_myPhraseTags.setPhraseTags(aHighlighter.getHighlightTags());
	}

	virtual void doHighlighting(QTextCharFormat &aFormat, bool bClear) const = 0;
	virtual bool enabled() const { return m_bEnabled; }
	virtual void setEnabled(bool bEnabled = true) { m_bEnabled = bEnabled; }

	const TPhraseTagList &getHighlightTags() const;
	void setHighlightTags(const TPhraseTagList &lstPhraseTags);

public slots:
	void clearPhraseTags();

protected:
	bool m_bEnabled;

private:
	// Guard class to keep me from accidentally accessing non-const functions and
	//		causing unintentional copying, as that can be expensive in large searches:
	class CMyPhraseTags {
	public:
		const TPhraseTagList &phraseTags() const { return m_lstPhraseTags; }
		void setPhraseTags(const TPhraseTagList &lstPhraseTags) { m_lstPhraseTags = lstPhraseTags; }

	private:
		TPhraseTagList m_lstPhraseTags;				// Tags to highlight
	} m_myPhraseTags;
};

// ============================================================================

class CSearchResultHighlighter : public CBasicHighlighter
{
	Q_OBJECT
public:
	explicit CSearchResultHighlighter(const TPhraseTagList &lstPhraseTags = TPhraseTagList(), QObject *parent = NULL)
		:	CBasicHighlighter(lstPhraseTags, parent)
	{
	}
	CSearchResultHighlighter(const TPhraseTag &aTag, QObject *parent = NULL)
		:	CBasicHighlighter(aTag, parent)
	{
	}

	virtual void doHighlighting(QTextCharFormat &aFormat, bool bClear) const;
};

// ============================================================================

class CCursorFollowHighlighter : public CBasicHighlighter
{
	Q_OBJECT
public:
	explicit CCursorFollowHighlighter(const TPhraseTagList &lstPhraseTags = TPhraseTagList(), QObject *parent = NULL)
		:	CBasicHighlighter(lstPhraseTags, parent)
	{
	}
	CCursorFollowHighlighter(const TPhraseTag &aTag, QObject *parent = NULL)
		:	CBasicHighlighter(aTag, parent)
	{
	}

	virtual void doHighlighting(QTextCharFormat &aFormat, bool bClear) const;
};

// ============================================================================

#endif // HIGHLIGHTER_H
