/****************************************************************************
**
** Copyright (C) 2013-2025 Donna Whisnant, a.k.a. Dewtronics.
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
#include "Highlighter.h"

// Forward declarations
class CPersistentSettings;

// ============================================================================

class CVerseTextRichifierTags
{
public:
	enum VERSE_TEMPLATE_TAGS_ENUM		// If changing this list, update CVerseTextRichifier() constructor!
	{
		VTTE_w,			// Word
		VTTE_M,			// Ps119 Hebrew Prefix
		VTTE_J,			// Words of Jesus Begin
		VTTE_j,			// Words of Jesus End
		VTTE_T,			// TransChangeAdded Begin
		VTTE_t,			// TransChangeAdded End
		VTTE_D,			// Divine Name Begin
		VTTE_d,			// Divine Name End
		VTTE_A,			// Anchor Begin
		VTTE_a,			// Anchor End
		VTTE_N,			// Inline Note Begin
		VTTE_n,			// Inline Note End
		VTTE_R,			// Search Results Begin
		VTTE_r,			// Search Results End
		VTTE_E,			// Search Results Excluded Begin
		VTTE_e,			// Search Results Excluded End
		VTTE_H,			// User Highlighter Begin
		VTTE_h,			// User Highlighter End
		VTTE_L,			// Lemma Begin (for completeness -- not used in richifier, see KJVDataParse)
		VTTE_l,			// Lemma End (for completeness -- not used in richifier, see KJVDataParse)
		// ----
		VTTE_COUNT
	};

	enum VerseWordTypes {
		VWT_None = 0x00,						// Default for normal word in verse
		VWT_TransChangeAdded = 0x01,			// Inside TransChangeAdded tag
		VWT_WordsOfJesus = 0x02,				// Inside Word of Jesus tag
		VWT_DivineName = 0x04,					// Inside Divine Name tag
		VWT_SearchResult = 0x08,				// Inside Search Results tag
		VWT_SearchResultExcluded = 0x10,		// Inside Search Results Excluded tag
	};
	Q_DECLARE_FLAGS(VerseWordTypeFlags, VerseWordTypes)

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
			m_strSearchResultsExcludedBegin("<font color=\"blue\" style=\"text-decoration: line-through;\">"),
			m_strSearchResultsExcludedEnd("</font>"),
			m_bShowPilcrowMarkers(true)
	{
		calcHash();
	}

	virtual ~CVerseTextRichifierTags()
	{

	}

	virtual void wordCallback(const QString &strWord, VerseWordTypeFlags nWordTypes) const { Q_UNUSED(strWord); Q_UNUSED(nWordTypes); }

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

	inline QString searchResultsExcludedBegin() const { return m_strSearchResultsExcludedBegin; }
	inline QString searchResultsExcludedEnd() const { return m_strSearchResultsExcludedEnd; }
	void setSearchResultsExcludedTags(const QString &strTagBegin, const QString &strTagEnd)
	{
		m_strSearchResultsExcludedBegin = strTagBegin;
		m_strSearchResultsExcludedEnd = strTagEnd;
		calcHash();
	}
	void setSearchResultsExcludedTagsByColor(const QColor &color)
	{
		if (color.isValid()) {
			setSearchResultsExcludedTags(QString("<font color=\"%1\" style=\"text-decoration: line-through;\">").arg(color.name()), "</font>");
		} else {
			setSearchResultsExcludedTags(QString(), QString());
		}
	}

	inline bool showPilcrowMarkers() const { return m_bShowPilcrowMarkers; }
	void setShowPilcrowMarkers(bool bShowPilcrowMarkers)
	{
		m_bShowPilcrowMarkers = bShowPilcrowMarkers;
		calcHash();
	}

	inline size_t hash() const { return m_nHash; }

protected:
	void setUsesHTML(bool bUsesHTML) { m_bUsesHTML = bUsesHTML; }

	void calcHash()
	{
		m_nHash = qHash(QString((m_bAddRichPs119HebrewPrefix ? 'M' : 'm')) +
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
						'E' + m_strSearchResultsExcludedBegin +
						'e' + m_strSearchResultsExcludedEnd +
						(m_bShowPilcrowMarkers ? 'P' : 'p'));
	}

private:
	bool m_bUsesHTML;
	size_t m_nHash;
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
	QString m_strSearchResultsExcludedBegin;
	QString m_strSearchResultsExcludedEnd;
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
		setSearchResultsExcludedTags(QString(), QString());
		setShowPilcrowMarkers(false);
	}
};

// ============================================================================

enum RichifierRenderOptions : uint32_t {
	RRO_None = 0x0,						// Default for no options
	RRO_AddWordAnchors = 0x1,			// Parsed text will have <a> word anchor tags added (words in template itself for tracking purposes)
	RRO_AddLinkAnchors = 0x2,			// Allows generation of text with external reference anchors, like Strong's references, etc.
	RRO_UseLemmas = 0x4,				// Render Lemmas interlinearly with verses
	RRO_UseWordSpans = 0x8,				// Output Word-Spans in HTML (this is implied when RRO_UseLemmas is specified)
	RRO_InlineFootnotes = 0x10,			// Render inline footnotes in verses even when the verse templates exclude inline notes
	RRO_EnableUserHighlighters = 0x20,	// Render user highlighters from user notes database
};
Q_DECLARE_FLAGS(RichifierRenderOptionFlags, RichifierRenderOptions)

class CVerseTextRichifier
{
private:
	class CRichifierBaton
	{
	public:
		CRichifierBaton(const CVerseTextRichifierTags &tags, const CBibleDatabase *pBibleDatabase,
						const CRelIndex &ndxRelative, const TPhraseTag &tagVerse,
						RichifierRenderOptionFlags flagsRRO, int *pWordCount = nullptr,
						const CBasicHighlighter *pSRHighlighter = nullptr,
						const CBasicHighlighter *pSRExclHighlighter = nullptr,
						const THighlighterTagMap *pUserHighlighters = nullptr)
			:	m_pBibleDatabase(pBibleDatabase),
				m_ndxCurrent(ndxRelative),
				m_tagVerse(tagVerse),
				m_flagsRRO(flagsRRO),
				// ----
				m_nStartWord(ndxRelative.word()),
				m_pWordCount(pWordCount),
				m_pSRHighlighter(pSRHighlighter),
				m_pSRExclHighlighter(pSRExclHighlighter),
				m_bOutput(false),
				m_bInTransChangeAdded(false),
				m_bInWordsOfJesus(false),
				m_bInSearchResult(false),
				m_bInSearchResultExcl(false),
				m_pCurrentLemma(nullptr),
				// ----
				m_pUserHighlighters(pUserHighlighters),
				// ----
				m_tags(tags)
		{
			Q_ASSERT(pBibleDatabase != nullptr);
			m_strVerseText.reserve(1024);					// Avoid reallocations
			m_ndxCurrent.setWord(0);						// Set ndxCurrent to be whole verse start, but nStartWord=Target First Word (set above)
			if (m_nStartWord == 1) m_nStartWord = 0;		// Starting at first word includes pretext prior to word
			if (m_nStartWord == 0) m_bOutput = true;

			// Setup "in highlighter" flags:
			if (m_pUserHighlighters) {
				for (THighlighterTagMap::const_iterator itrHighlighters = m_pUserHighlighters->cbegin(); itrHighlighters != m_pUserHighlighters->cend(); ++itrHighlighters) {
					m_mapInHighlighter[itrHighlighters->first] = false;
				}
			}
		}

		bool usesHTML() const { return m_tags.usesHTML(); }
		bool renderOption(RichifierRenderOptionFlags flagsRRO) const { return ((m_flagsRRO & flagsRRO) != 0); }

		const CBibleDatabase *m_pBibleDatabase;
		CRelIndex m_ndxCurrent;								// Current RelIndex within verse (updates in parser with verse pointer)
		TPhraseTag m_tagVerse;								// Span of verse being parsed/richified
		RichifierRenderOptionFlags m_flagsRRO;				// Rendering Flags
		// ----
		QString m_strVerseText;								// Verse Text being built
		QString m_strPrewordStack;							// Verse Text to save and push at the beginning of the next word
		QString m_strDivineNameFirstLetterParseText;		// Special First-Letter Markup Text for Divine Name
		uint32_t m_nStartWord;								// Set to the word to start parse on from ndxRelative on initial call (0 and 1 are both start of verse)
		int *m_pWordCount;									// Pointer to Number of words of verse to output.  We output while the integer pointed to by this is >0.
		const CBasicHighlighter *m_pSRHighlighter;			// Search Results word highligher if set
		const CBasicHighlighter *m_pSRExclHighlighter;		// Excluded Search Results word highlighter if set
		bool m_bOutput;										// True when outputting text
		bool m_bInTransChangeAdded;							// True when we are inside translation change addition text (i.e. italics)
		bool m_bInWordsOfJesus;								// True when we are inside Words of Jesus text
		bool m_bInSearchResult;								// True when we are inside intersection of m_pSRHighlighter, f->t triggers writing the begin tag, t->f triggers writing the end tag
		bool m_bInSearchResultExcl;							// True when we are inside intersection of m_pSRExclHighlighter, f->t triggers writing the begin tag, t->f triggers writing the end tag
		const CLemmaEntry *m_pCurrentLemma;					// Pointer to the Lemma currently being processed or null if no Lemma exists for the current word/tag
		// ----
		const THighlighterTagMap *m_pUserHighlighters;		// Collection of user highlighters to process
		typedef std::map<QString, bool> THighlighterTagMapFlags;
		THighlighterTagMapFlags m_mapInHighlighter;			// InHighlighting flags for each user highlighter processed
		// ----
		const CVerseTextRichifierTags &m_tags;				// Tags from the caller used for parsing and callback
	};

	typedef QString (*FXlateText)(const CRichifierBaton &parseBaton);

	CVerseTextRichifier(CRichifierBaton &parseBaton, CVerseTextRichifierTags::VERSE_TEMPLATE_TAGS_ENUM nMatchChar,
						const CVerseTextRichifier *pRichNext = nullptr);
	CVerseTextRichifier(CRichifierBaton &parseBaton, CVerseTextRichifierTags::VERSE_TEMPLATE_TAGS_ENUM nMatchChar,
						const CVerseEntry *pVerse, const CVerseTextRichifier *pRichNext = nullptr);

	~CVerseTextRichifier();

	void parse(const QString &strNodeIn) const;
	void startLemma(const CRelIndex &ndxWord) const;	// Starts the lemma main span output for a lemma
	void finishLemma() const;			// Finishes off the main span of the lemma and writes the rest of the lemma too (i.e. strongs, morph, interlinear, etc)
	void startWordSpan() const;			// Starts a word span for a specific word or a lemma word group
	void finishWordSpan() const;		// Finishes off a word span for a specific word or a lemma word group
	void startHighlighter(const CBasicHighlighter &highlighter, bool &bInHighlighterFlag) const;		// Starts a highlighter output
	void finishHighlighter(const CBasicHighlighter &highlighter, bool &bInHighlighterFlag) const;	// Finishes highlighter output
	void writeXlateText() const;		// Writes fncXlateText generated output
	bool isStartOperator() const { return m_chrMatchChar.isUpper(); }

protected:
	virtual void pushWordToVerseText(const QString &strWord) const;

public:
	static QString parse(const CRelIndex &ndxRelative, const CBibleDatabase *pBibleDatabase, const CVerseEntry *pVerse,
							const CVerseTextRichifierTags &tags = CVerseTextRichifierTags(),
							RichifierRenderOptionFlags flagsRRO = RichifierRenderOptionFlags(),
							int *pWordCount = nullptr, const CBasicHighlighter *pSRHighlighter = nullptr,
							const CBasicHighlighter *pSRExclHighlighter = nullptr);

private:
	CRichifierBaton &m_parseBaton;
	// ----
	const CVerseTextRichifier *m_pRichNext;
	CVerseTextRichifierTags::VERSE_TEMPLATE_TAGS_ENUM m_vtteMatch;
	QChar m_chrMatchChar;
	const CVerseEntry *m_pVerse;		// This will only be set on CVerseTextRichifier objects doing 'w' or word parsing
	FXlateText m_fncXlateText;
};

// ============================================================================

#endif	// VERSE_RICHIFIER_H

