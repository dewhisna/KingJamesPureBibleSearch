/****************************************************************************
**
** Copyright (C) 2013 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef VERSE_RICHIFIER_H
#define VERSE_RICHIFIER_H

#include <QString>
#include <QHash>
#include <QColor>

#include "dbstruct.h"

// Forward declarations
class CPersistentSettings;

// ============================================================================

class CVerseTextRichifierTags
{
public:
	CVerseTextRichifierTags()
		:	m_bUsesHTML(true),
			m_nHash(0),
			m_bAddRichPs119HebrewPrefix(true),
			m_strTransChangeAddedBegin("<i>"),
			m_strTransChangeAddedEnd("</i>"),
			m_strWordsOfJesusBegin("<font color=\"red\">"),
			m_strWordsOfJesusEnd("</font> "),
//			m_strDivineNameBegin("<b>"),
//			m_strDivineNameEnd("</b>")
			m_strDivineNameBegin("<font size=\"-1\">"),
			m_strDivineNameEnd("</font>"),
			m_strInlineNoteBegin("("),
			m_strInlineNoteEnd(")"),
			m_strSearchResultsBegin("<font color=\"blue\">"),
			m_strSearchResultsEnd("</font>"),
			m_bShowPilcrowMarkers(true)
	{
		calcHash();
	}

	~CVerseTextRichifierTags()
	{

	}

	bool usesHTML() const { return m_bUsesHTML; }

	void setFromPersistentSettings(const CPersistentSettings &aPersistentSettings, bool bCopyOptions = false);	// If bCopyOptions==true, DisplayOptions will be used

	inline bool addRichPs119HebrewPrefix() const { return m_bAddRichPs119HebrewPrefix; }
	void setAddRichPs119HebrewPrefix(bool bAddRichPs119HebrewPrefix)
	{
		m_bAddRichPs119HebrewPrefix = bAddRichPs119HebrewPrefix;
		calcHash();
	}

	inline QString transChangeAddedBegin() const { return m_strTransChangeAddedBegin; }
	inline QString transChangeAddedEnd() const { return m_strTransChangeAddedEnd; }
	void setTransChangeAddedTags(const QString &strTagBegin, const QString &strTagEnd)
	{
		m_strTransChangeAddedBegin = strTagBegin;
		m_strTransChangeAddedEnd = strTagEnd;
		calcHash();
	}

	inline QString wordsOfJesusBegin() const { return m_strWordsOfJesusBegin; }
	inline QString wordsOfJesusEnd() const { return m_strWordsOfJesusEnd; }
	void setWordsOfJesusTags(const QString &strTagBegin, const QString &strTagEnd)
	{
		m_strWordsOfJesusBegin = strTagBegin;
		m_strWordsOfJesusEnd = strTagEnd;
		calcHash();
	}
	void setWordsOfJesusTagsByColor(const QColor &color)
	{
		if (color.isValid()) {
			setWordsOfJesusTags(QString("<font color=\"%1\">").arg(color.name()), "</font> ");
		} else {
			setWordsOfJesusTags(QString(), QString());
		}
	}

	inline QString divineNameBegin() const { return m_strDivineNameBegin; }
	inline QString divineNameEnd() const { return m_strDivineNameEnd; }
	void setDivineNameTags(const QString &strTagBegin, const QString &strTagEnd)
	{
		m_strDivineNameBegin = strTagBegin;
		m_strDivineNameEnd = strTagEnd;
		calcHash();
	}

	inline QString inlineNoteBegin() const { return m_strInlineNoteBegin; }
	inline QString inlineNoteEnd() const { return m_strInlineNoteEnd; }
	void setInlineNoteTags(const QString &strTagBegin, const QString &strTagEnd)
	{
		m_strInlineNoteBegin = strTagBegin;
		m_strInlineNoteEnd = strTagEnd;
		calcHash();
	}

	inline QString searchResultsBegin() const { return m_strSearchResultsBegin; }
	inline QString searchResultsEnd() const { return m_strSearchResultsEnd; }
	void setSearchResultsTags(const QString &strTagBegin, const QString &strTagEnd)
	{
		m_strSearchResultsBegin = strTagBegin;
		m_strSearchResultsEnd = strTagEnd;
		calcHash();
	}
	void setSearchResultsTagsByColor(const QColor &color)
	{
		if (color.isValid()) {
			setSearchResultsTags(QString("<font color=\"%1\">").arg(color.name()), "</font>");
		} else {
			setSearchResultsTags(QString(), QString());
		}
	}

	inline bool showPilcrowMarkers() const { return m_bShowPilcrowMarkers; }
	void setShowPilcrowMarkers(bool bShowPilcrowMarkers)
	{
		m_bShowPilcrowMarkers = bShowPilcrowMarkers;
		calcHash();
	}

	inline uint hash() const { return m_nHash; }

protected:
	void setUsesHTML(bool bUsesHTML) { m_bUsesHTML = bUsesHTML; }

	void calcHash()
	{
		m_nHash = qHash((m_bAddRichPs119HebrewPrefix ? 'M' : 'm') +
						'T' + m_strTransChangeAddedBegin +
						't' + m_strTransChangeAddedEnd +
						'J' + m_strWordsOfJesusBegin +
						'j' + m_strWordsOfJesusEnd +
						'D' + m_strDivineNameBegin +
						'd' + m_strDivineNameEnd +
						'N' + m_strInlineNoteBegin +
						'n' + m_strInlineNoteEnd +
						'R' + m_strSearchResultsBegin +
						'r' + m_strSearchResultsEnd +
						(m_bShowPilcrowMarkers ? 'P' : 'p'));
	}

private:
	bool m_bUsesHTML;
	uint m_nHash;
	bool m_bAddRichPs119HebrewPrefix;
	QString m_strTransChangeAddedBegin;
	QString m_strTransChangeAddedEnd;
	QString m_strWordsOfJesusBegin;
	QString m_strWordsOfJesusEnd;
	QString m_strDivineNameBegin;
	QString m_strDivineNameEnd;
	QString m_strInlineNoteBegin;
	QString m_strInlineNoteEnd;
	QString m_strSearchResultsBegin;
	QString m_strSearchResultsEnd;
	bool m_bShowPilcrowMarkers;
};

class CVerseTextPlainRichifierTags : public CVerseTextRichifierTags
{
public:
	CVerseTextPlainRichifierTags()
		:	CVerseTextRichifierTags()
	{
		setUsesHTML(false);
		setAddRichPs119HebrewPrefix(false);
		setTransChangeAddedTags("[", "]");
		setWordsOfJesusTags(QString(), QString());
		setDivineNameTags(QString(), QString());
		setSearchResultsTags(QString(), QString());
		setShowPilcrowMarkers(false);
	}
};

// ============================================================================

class CVerseTextRichifier
{
private:
	CVerseTextRichifier(const QChar &chrMatchChar, const QString &strXlateText, const CVerseTextRichifier *pRichNext = NULL);
	CVerseTextRichifier(const QChar &chrMatchChar, const CVerseEntry *pVerse, const CVerseTextRichifier *pRichNext = NULL, bool bAddAnchors = false, bool bUseLemmas = false);

	~CVerseTextRichifier();

	class CRichifierBaton
	{
	public:
		CRichifierBaton(const CBibleDatabase *pBibleDatabase, const CRelIndex &ndxRelative, const QString &strTemplate, bool bUsesHTML, int *pWordCount = NULL, const CBasicHighlighter *pHighlighter = NULL)
			:	m_pBibleDatabase(pBibleDatabase),
				m_ndxCurrent(ndxRelative),
				m_strTemplate(strTemplate),
				m_bUsesHTML(bUsesHTML),
				// ----
				m_nStartWord(ndxRelative.word()),
				m_pWordCount(pWordCount),
				m_pHighlighter(pHighlighter),
				m_bOutput(false),
				m_bInSearchResult(false),
				m_pCurrentLemma(nullptr)
		{
			assert(pBibleDatabase != NULL);
			m_strVerseText.reserve(1024);					// Avoid reallocations
			m_ndxCurrent.setWord(0);						// Set ndxCurrent to be whole verse start, but nStartWord=Target First Word (set above)
			if (m_nStartWord == 1) m_nStartWord = 0;		// Starting at first word includes pretext prior to word
			if (m_nStartWord == 0) m_bOutput = true;
		}

		const CBibleDatabase *m_pBibleDatabase;
		CRelIndex m_ndxCurrent;
		QString m_strTemplate;								// Verse Template being parsed -- will be identical to the one from CVerseEntry if not doing SearchResults, or modified if we are
		bool m_bUsesHTML;									// True if the CVerseTextRichifierTags being used supports HTML tags
		// ----
		QString m_strVerseText;								// Verse Text being built
		QString m_strPrewordStack;							// Verse Text to save and push at the beginning of the next word
		QString m_strDivineNameFirstLetterParseText;		// Special First-Letter Markup Text for Divine Name
		uint32_t m_nStartWord;								// Set to the word to start parse on from ndxRelative on initial call (0 and 1 are both start of verse)
		int *m_pWordCount;									// Pointer to Number of words of verse to output.  We output while the integer pointed to by this is >0.
		const CBasicHighlighter *m_pHighlighter;			// Search Results (or other) word highligher if set
		bool m_bOutput;										// True when outputting text
		bool m_bInSearchResult;								// True when we are inside intersection of m_lstTagsSearchResults, f->t triggers writing the begin tag, t->f triggers writing the end tag
		const CLemmaEntry *m_pCurrentLemma;					// Pointer to the Lemma currently being processed or null if no Lemma exists for the current word/tag
	};

	void parse(CRichifierBaton &parseBaton, const QString &strNodeIn = QString()) const;
	void writeLemma(CRichifierBaton &parseBaton) const;
	bool isStartOperator() const { return m_chrMatchChar.isUpper(); }

public:
	static QString parse(const CRelIndex &ndxRelative, const CBibleDatabase *pBibleDatabase, const CVerseEntry *pVerse,
							const CVerseTextRichifierTags &tags = CVerseTextRichifierTags(), bool bAddAnchors = false,
							int *pWordCount = NULL, const CBasicHighlighter *pHighlighter = NULL, bool bUseLemmas = false);

private:
	const CVerseTextRichifier *m_pRichNext;
	const QChar m_chrMatchChar;
	const CVerseEntry *m_pVerse;
	QString m_strXlateText;
	bool m_bAddAnchors;
	bool m_bUseLemmas;
};

// ============================================================================

#endif	// VERSE_RICHIFIER_H

