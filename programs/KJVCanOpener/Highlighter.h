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
			m_lstPhraseTags(lstPhraseTags),
			m_bEnabled(true)
	{
	}
	CBasicHighlighter(const TPhraseTag &aTag, QObject *parent = NULL)
		:	QObject(parent),
			m_bEnabled(true)
	{
		m_lstPhraseTags.append(aTag);
	}

	virtual void doHighlighting(QTextCharFormat &aFormat, bool bClear) const = 0;
	virtual bool enabled() const { return m_bEnabled; }
	virtual void setEnabled(bool bEnabled = true) { m_bEnabled = bEnabled; }

	const TPhraseTagList &getHighlightTags() const;
	void setHighlightTags(const TPhraseTagList &lstPhraseTags);

public slots:
	void clearPhraseTags();

protected:
	TPhraseTagList m_lstPhraseTags;				// Tags to highlight
	bool m_bEnabled;
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
