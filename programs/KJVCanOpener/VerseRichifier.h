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


// ============================================================================

class CVerseTextRichifierTags
{
public:
	CVerseTextRichifierTags()
		:	m_nHash(0),
			m_bAddRichPs119HebrewPrefix(true),
			m_strTransChangeAddedBegin("<i>"),
			m_strTransChangeAddedEnd("</i>"),
			m_strWordsOfJesusBegin("<font color=\"red\"> "),
			m_strWordsOfJesusEnd("</font> "),
//			m_strDivideNameBegin("<b>"),
//			m_strDivideNameEnd("</b>")
			m_strDivideNameBegin("<font size=\"-1\">"),
			m_strDivideNameEnd("</font>")
	{
		calcHash();
	}

	~CVerseTextRichifierTags()
	{

	}

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
	void setWordsOfJesusTagsByColor(const QColor &color) {
		setWordsOfJesusTags(QString("<font color=\"%1\"> ").arg(color.name()), "</font> ");
	}

	inline QString divineNameBegin() const { return m_strDivideNameBegin; }
	inline QString divineNameEnd() const { return m_strDivideNameEnd; }
	void setDivineNameTags(const QString &strTagBegin, const QString &strTagEnd)
	{
		m_strDivideNameBegin = strTagBegin;
		m_strDivideNameEnd = strTagEnd;
		calcHash();
	}

	inline uint hash() const { return m_nHash; }

protected:
	void calcHash()
	{
		m_nHash = qHash((m_bAddRichPs119HebrewPrefix ? 'M' : 'm') +
						'T' + m_strTransChangeAddedBegin +
						't' + m_strTransChangeAddedEnd +
						'J' + m_strWordsOfJesusBegin +
						'j' + m_strWordsOfJesusEnd +
						'D' + m_strDivideNameBegin +
						'd' + m_strDivideNameEnd);
	}

private:
	uint m_nHash;
	bool m_bAddRichPs119HebrewPrefix;
	QString m_strTransChangeAddedBegin;
	QString m_strTransChangeAddedEnd;
	QString m_strWordsOfJesusBegin;
	QString m_strWordsOfJesusEnd;
	QString m_strDivideNameBegin;
	QString m_strDivideNameEnd;
};

class CVerseTextPlainRichifierTags : public CVerseTextRichifierTags
{
public:
	CVerseTextPlainRichifierTags()
		:	CVerseTextRichifierTags()
	{
		setAddRichPs119HebrewPrefix(false);
		setTransChangeAddedTags("[", "]");
		setWordsOfJesusTags(QString(), QString());
		setDivineNameTags(QString(), QString());
	}
};

// ============================================================================

class CVerseTextRichifier
{
private:
	CVerseTextRichifier(const QChar &chrMatchChar, const QString &strXlateText, const CVerseTextRichifier *pRichNext = NULL);
	CVerseTextRichifier(const QChar &chrMatchChar, const CVerseEntry *pVerse, const CVerseTextRichifier *pRichNext = NULL, bool bAddAnchors = false);

	~CVerseTextRichifier();

	class CRichifierBaton
	{
	public:
		CRichifierBaton(const CBibleDatabase *pBibleDatabase, const CRelIndex &ndxRelative, int *pWordCount = NULL)
			:	m_pBibleDatabase(pBibleDatabase),
				m_ndxCurrent(ndxRelative),
				m_nStartWord(ndxRelative.word()),
				m_pWordCount(pWordCount),
				m_bOutput(false)
		{
			assert(pBibleDatabase != NULL);
			m_strVerseText.reserve(1024);					// Avoid reallocations
			m_ndxCurrent.setWord(0);						// Set ndxCurrent to be whole verse start, but nStartWord=Target First Word (set above)
			if (m_nStartWord == 1) m_nStartWord = 0;		// Starting at first word includes pretext prior to word
			if (m_nStartWord == 0) m_bOutput = true;
		}

		QString m_strVerseText;								// Verse Text being built
		QString m_strDivineNameFirstLetterParseText;		// Special First-Letter Markup Text for Divine Name
		const CBibleDatabase *m_pBibleDatabase;
		CRelIndex m_ndxCurrent;
		uint32_t m_nStartWord;								// Set to the word to start parse on from ndxRelative on initial call (0 and 1 are both start of verse)
		int *m_pWordCount;									// Pointer to Number of words of verse to output.  We output while the integer pointed to by this is >0.
		bool m_bOutput;										// True when outputting text
	};

	void parse(CRichifierBaton &parseBaton, const QString &strNodeIn = QString()) const;

public:
	static QString parse(const CRelIndex &ndxRelative, const CBibleDatabase *pBibleDatabase, const CVerseEntry *pVerse,
							const CVerseTextRichifierTags &tags = CVerseTextRichifierTags(), bool bAddAnchors = false, int *pWordCount = NULL);

private:
	const CVerseTextRichifier *m_pRichNext;
	const QChar m_chrMatchChar;
	const CVerseEntry *m_pVerse;
	QString m_strXlateText;
	bool m_bAddAnchors;
};

// ============================================================================

#endif	// VERSE_RICHIFIER_H

