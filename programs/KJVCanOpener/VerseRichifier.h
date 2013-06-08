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

#include "dbstruct.h"

// ============================================================================

class CVerseTextRichifierTags
{
public:
	CVerseTextRichifierTags()
		:	m_bAddRichPs119HebrewPrefix(true),
			m_strTransChangeAddedBegin("<i>"),
			m_strTransChangeAddedEnd("</i>"),
			m_strWordsOfJesusBegin("<font color=\"red\"> "),
			m_strWordsOfJesusEnd("</font> "),
//			m_strDivideNameBegin("<b>"),
//			m_strDivideNameEnd("</b>")
			m_strDivideNameBegin("<font size=\"-1\">"),
			m_strDivideNameEnd("</font>")
	{

	}

	~CVerseTextRichifierTags()
	{

	}

	bool addRichPs119HebrewPrefix() const { return m_bAddRichPs119HebrewPrefix; }
	void setAddRichPs119HebrewPrefix(bool bAddRichPs119HebrewPrefix)
	{
		m_bAddRichPs119HebrewPrefix = bAddRichPs119HebrewPrefix;
	}

	QString transChangeAddedBegin() const { return m_strTransChangeAddedBegin; }
	QString transChangeAddedEnd() const { return m_strTransChangeAddedEnd; }
	void setTransChangeAddedTags(const QString &strTagBegin, const QString &strTagEnd)
	{
		m_strTransChangeAddedBegin = strTagBegin;
		m_strTransChangeAddedEnd = strTagEnd;
	}

	QString wordsOfJesusBegin() const { return m_strWordsOfJesusBegin; }
	QString wordsOfJesusEnd() const { return m_strWordsOfJesusEnd; }
	void setWordsOfJesusTags(const QString &strTagBegin, const QString &strTagEnd)
	{
		m_strWordsOfJesusBegin = strTagBegin;
		m_strWordsOfJesusEnd = strTagEnd;
	}

	QString divineNameBegin() const { return m_strDivideNameBegin; }
	QString divineNameEnd() const { return m_strDivideNameEnd; }
	void setDivineNameTags(const QString &strTagBegin, const QString &strTagEnd)
	{
		m_strDivideNameBegin = strTagBegin;
		m_strDivideNameEnd = strTagEnd;
	}

protected:
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
		CRichifierBaton(const CBibleDatabase *pBibleDatabase, const CRelIndex &ndxRelative)
			:	m_pBibleDatabase(pBibleDatabase),
				m_ndxCurrent(ndxRelative)
		{
			assert(pBibleDatabase != NULL);
			m_strVerseText.reserve(1024);					// Avoid reallocations
		}

		QString m_strVerseText;								// Verse Text being built
		QString m_strDivineNameFirstLetterParseText;		// Special First-Letter Markup Text for Divine Name
		const CBibleDatabase *m_pBibleDatabase;
		CRelIndex m_ndxCurrent;
	};

	void parse(CRichifierBaton &parseBaton, const QString &strNodeIn = QString()) const;

public:
	static QString parse(const CRelIndex &ndxRelative, const CBibleDatabase *pBibleDatabase, const CVerseEntry *pVerse, const CVerseTextRichifierTags &tags = CVerseTextRichifierTags(), bool bAddAnchors = false);

private:
	const CVerseTextRichifier *m_pRichNext;
	const QChar m_chrMatchChar;
	const CVerseEntry *m_pVerse;
	QString m_strXlateText;
	bool m_bAddAnchors;
};

// ============================================================================

#endif	// VERSE_RICHIFIER_H

