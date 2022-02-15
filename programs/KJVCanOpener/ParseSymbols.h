/****************************************************************************
**
** Copyright (C) 2013-2020 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef PARSE_SYMBOLS_H
#define PARSE_SYMBOLS_H

#include <QChar>
#include <QString>

// For processing hyphenated words, the following symbols will be treated
//	as hyphens and rolled into the "-" symbol for processing.  Words with
//	only hyphen differences will be added to the base word as a special
//	alternate form, allowing users to search them with or without hyphen
//	sensitivity:
extern const QString g_strHyphens;

// For processing words with apostrophes, the following symbols will be treated
//	as apostrophes and rolled into the "'" symbol for processing.  Words with
//	only apostrophe differences will be added to the base word as a special
//	alternate form, allowing users to search them with or without the apostrophe:
extern const QString g_strApostrophes;

// Ascii Word characters -- these will be kept in words as-is and includes
//	alphanumerics.  Hyphen and apostrophe are kept too, but by the rules
//	above, not here.  Non-Ascii (high UTF8 values) are also kept, but have
//	rules of their own:
extern const QString g_strAsciiWordChars;

// Non-Word Non-Ascii characters -- these are non-Ascii characters the do
//	do automatically apply as a word -- things like quotes, etc:
extern const QString g_strNonAsciiNonWordChars;

// Pilcrow paragraph marker
extern const QChar g_chrPilcrow;

// ============================================================================

namespace StringParse {
	QString decompose(const QString &strWord, bool bRemoveHyphens);			// Word decompose() function to breakdown and remove accents from words for searching purposes
	QString deLigature(const QString &strWord);								// Decompose Ligatures into their constituent parts (decompose already does this too)
	QString deApostrHyphen(const QString &strWord, bool bRemoveHyphens);	// Decompose Apostrophes and Hyphens so matches work correctly and yet rendered text can have the rich set.  (decompose already does this too)
	QString deApostrophe(const QString &strWord, bool bRemove = false);		// Decompose/remove Apostrophes only
	QString deHyphen(const QString &strWord, bool bRemove = false);			// Decompose/remove Hyphens only
};

// ============================================================================

#endif	// PARSE_SYMBOLS_H
