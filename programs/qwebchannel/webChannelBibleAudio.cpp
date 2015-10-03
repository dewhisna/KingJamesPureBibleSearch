/****************************************************************************
**
** Copyright (C) 2015 Donna Whisnant, a.k.a. Dewtronics.
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

#include "webChannelBibleAudio.h"
#include "dbDescriptors.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

// ============================================================================

namespace {

#define NUM_BK 66
#define NUMBERS_BOOK_NUM 04
#define PSALMS_BOOK_NUM 19
#define PHILEMON_BOOK_NUM 57
#define _2JOHN_BOOK_NUM 63
#define _3JOHN_BOOK_NUM 64
#define JUDE_BOOK_NUM 65

	// Book names as they appear on the FCBH recording files:
	const QString g_arrconstrFCBHBooks[NUM_BK] = {
		"Genesis",
		"Exodus",
		"Leviticus",
		"Numbers",
		"Deuteronomy",
		"Joshua",
		"Judges",
		"Ruth",
		"1Samuel",
		"2Samuel",
		"1Kings",
		"2Kings",
		"1Chronicles",
		"2Chronicles",
		"Ezra",
		"Nehemiah",
		"Esther",
		"Job",
		"Psalms",
		"Proverbs",
		"Ecclesiastes",
		"SongofSongs",
		"Isaiah",
		"Jeremiah",
		"Lamentations",
		"Ezekiel",
		"Daniel",
		"Hosea",
		"Joel",
		"Amos",
		"Obadiah",
		"Jonah",
		"Micah",
		"Nahum",
		"Habakkuk",
		"Zephaniah",
		"Haggai",
		"Zechariah",
		"Malachi",
		"Matthew",
		"Mark",
		"Luke",
		"John",
		"Acts",
		"Romans",
		"1Corinthians",
		"2Corinthians",
		"Galatians",
		"Ephesians",
		"Philippians",
		"Colossians",
		"1Thess",
		"2Thess",
		"1Timothy",
		"2Timothy",
		"Titus",
		"Philemon",
		"Hebrews",
		"James",
		"1Peter",
		"2Peter",
		"1John",
		"2John",
		"3John",
		"Jude",
		"Revelation"
	};

	const QString g_constrFCBHNonDramaURL = "http://audios.dewtronics.com/KingJamesBible/ON_NonDrama/%1";
	const QString g_constrFCBHDramaURL = "http://audios.dewtronics.com/KingJamesBible/ON_Drama/%1";

	// ------------------------------------------------------------------------

	// Book names as they appear on the DanWagner recording files:
	const QString g_arrconstrDWBooks[NUM_BK] = {
		"Genesis",
		"Exodus",
		"Leviticus",
		"Numbers",
		"Deuteronomy",
		"Joshua",
		"Judges",
		"Ruth",
		"1Samuel",
		"2Samuel",
		"1Kings",
		"2Kings",
		"1Chronicles",
		"2Chronicles",
		"Ezra",
		"Nehemiah",
		"Esther",
		"Job",
		"Psalms",
		"Proverbs",
		"Ecclesiastes",
		"Song_of_Solomon",
		"Isaiah",
		"Jeremiah",
		"Lamentations",
		"Ezekiel",
		"Daniel",
		"Hosea",
		"Joel",
		"Amos",
		"Obadiah",
		"Jonah",
		"Micah",
		"Nahum",
		"Habakkuk",
		"Zephaniah",
		"Haggai",
		"Zechariah",
		"Malachi",
		"Matthew",
		"Mark",
		"Luke",
		"John",
		"Acts",
		"Romans",
		"1Corinthians",
		"2Corinthians",
		"Galatians",
		"Ephesians",
		"Philippians",
		"Colossians",
		"1Thessalonians",
		"2Thessalonians",
		"1Timothy",
		"2Timothy",
		"Titus",
		"Philemon",
		"Hebrews",
		"James",
		"1Peter",
		"2Peter",
		"1John",
		"2John",
		"3John",
		"Jude",
		"Revelation"
	};

	const QString g_constrDWURL = "http://audios.dewtronics.com/KingJamesBible/DanWagner/%1";

	// ------------------------------------------------------------------------

	// Book names as they appear on the StephenJohnston recording files:
	const QString g_arrconstrSJBooks[NUM_BK] = {
		"Gen",
		"Exo",
		"Lev",
		"Num",
		"Deu",
		"Jos",
		"Jdg",
		"Rth",
		"1Sa",
		"2Sa",
		"1Ki",
		"2Ki",
		"1Ch",
		"2Ch",
		"Ezr",
		"Neh",
		"Est",
		"Job",
		"Psa",
		"Pro",
		"Ecc",
		"Son",
		"Isa",
		"Jer",
		"Lam",
		"Eze",
		"Dan",
		"Hos",
		"Joe",
		"Amo",
		"Oba",
		"Jon",
		"Mic",
		"Nah",
		"Hab",
		"Zep",
		"Hag",
		"Zec",
		"Mal",
		"Mat",
		"Mar",
		"Luk",
		"Joh",
		"Act",
		"Rom",
		"1Co",
		"2Co",
		"Gal",
		"Eph",
		"Php",
		"Col",
		"1Th",
		"2Th",
		"1Ti",
		"2Ti",
		"Tts",
		"Phm",
		"Heb",
		"Jam",
		"1Pe",
		"2Pe",
		"1Jo",
		"2Jo",
		"3Jo",
		"Jde",
		"Rev"
	};

	const QString g_constrSJURL = "http://audios.dewtronics.com/KingJamesBible/StephenJohnston/%1";

	// ------------------------------------------------------------------------

	// Book names as they appear on the ChristopherGlyn recording files:
	const QString g_arrconstrCGBooks[NUM_BK] = {
		"Genesis",
		"Exodus",
		"Leviticus",
		"Numbers",
		"Deuteronomy",
		"Joshua",
		"Judges",
		"Ruth",
		"1-Samuel",
		"2-Samuel",
		"1-Kings",
		"2-Kings",
		"1-Chronicles",
		"2-Chronicles",
		"Ezra",
		"Nehemiah",
		"Esther",
		"Job",
		"Psalm",
		"Proverbs",
		"Ecclesiastes",
		"Song_of_Solomon",
		"Isaiah",
		"Jeremiah",
		"Lamentations",
		"Ezekiel",
		"Daniel",
		"Hosea",
		"Joel",
		"Amos",
		"Obadiah",
		"Jonah",
		"Micah",
		"Nahum",
		"Habakkuk",
		"Zephaniah",
		"Haggai",
		"Zechariah",
		"Malachi",
		"Matthew",
		"Mark",
		"Luke",
		"John",
		"Acts",
		"Romans",
		"1Corinthians",
		"2Corinthians",
		"Galatians",
		"Ephesians",
		"Philippians",
		"Colossians",
		"1Thessalonians",
		"2Thessalonians",
		"1Timothy",
		"2Timothy",
		"Titus",
		"Philemon",
		"Hebrews",
		"James",
		"1Peter",
		"2Peter",
		"1John",
		"2John",
		"3John",
		"Jude",
		"Revelation"
	};

	const QString g_constrCGURL = "http://audios.dewtronics.com/KingJamesBible/ChristopherGlyn/%1";

	// ------------------------------------------------------------------------

}	// Namespace

// ============================================================================


CWebChannelBibleAudio::CWebChannelBibleAudio()
{

}

CWebChannelBibleAudio::~CWebChannelBibleAudio()
{

}

CWebChannelBibleAudio *CWebChannelBibleAudio::instance()
{
	static CWebChannelBibleAudio g_wcba;
	return &g_wcba;
}

QString CWebChannelBibleAudio::urlsForChapterAudio(const CBibleDatabasePtr pBibleDatabase, const CRelIndex &ndxRel, BibleAudioSourcesFlags flagsBAS)
{
	assert(!pBibleDatabase.isNull());

	CRelIndex ndxDecolophonated(ndxRel);
	if (ndxDecolophonated.isColophon()) {
		const CBookEntry *pBook = pBibleDatabase->bookEntry(ndxDecolophonated);
		if (pBook) {
			ndxDecolophonated.setChapter(pBook->m_nNumChp);
		}
	}

	QJsonArray arrBibleAudioList;

	if (ndxDecolophonated.isSet()) {
		int nTst = CRefCountCalc(pBibleDatabase.data(), CRefCountCalc::RTE_TESTAMENT, ndxDecolophonated).ofBible().first;
		int nBkTst = CRefCountCalc(pBibleDatabase.data(), CRefCountCalc::RTE_BOOK, ndxDecolophonated).ofTestament().first;
		int nBk = CRefCountCalc(pBibleDatabase.data(), CRefCountCalc::RTE_BOOK, ndxDecolophonated).ofBible().first;
		int nChp = CRefCountCalc(pBibleDatabase.data(), CRefCountCalc::RTE_CHAPTER, ndxDecolophonated).ofBook().first;

		bool bKJVValid = (((nTst > 0) && (nTst <= 2)) && (nBkTst != 0) && ((nBk > 0) && (nBk <= NUM_BK)));
		if ((pBibleDatabase->compatibilityUUID().compare(bibleDescriptor(BDE_KJV).m_strUUID, Qt::CaseInsensitive) != 0) &&
			(pBibleDatabase->compatibilityUUID().compare(bibleDescriptor(BDE_KJVPCE).m_strUUID, Qt::CaseInsensitive) != 0) &&
			(pBibleDatabase->compatibilityUUID().compare(bibleDescriptor(BDE_KJVA).m_strUUID, Qt::CaseInsensitive) != 0) &&
			(pBibleDatabase->compatibilityUUID().compare(bibleDescriptor(BDE_KJV1611).m_strUUID, Qt::CaseInsensitive) != 0) &&
			(pBibleDatabase->compatibilityUUID().compare(bibleDescriptor(BDE_KJV1611A).m_strUUID, Qt::CaseInsensitive) != 0))
			bKJVValid = false;

		if (bKJVValid) {
			// Faith Comes By Hearing:
			QString strFCBHBookName = QString("%1").arg(g_arrconstrFCBHBooks[nBk-1], -12, QChar('_'));
			QString strFCBHBkTst = QString("%1%2").arg((nTst == 1) ? "A" : "B").arg(nBkTst, 2, 10, QChar('0'));
			QString strFCBHChp = ((nBk == PSALMS_BOOK_NUM) ?
								QString("%1").arg(nChp, 3, 10, QChar('0')) :
								QString("_%1").arg(nChp, 2, 10, QChar('0')));
			QString strFCBHNonDrama = QString("%1__%2_%3ENGKJVC1DA.mp3").arg(strFCBHBkTst).arg(strFCBHChp).arg(strFCBHBookName);
			QString strFCBHDrama = QString("%1__%2_%3ENGKJVC2DA.mp3").arg(strFCBHBkTst).arg(strFCBHChp).arg(strFCBHBookName);
			if (flagsBAS & BAS_FCBH_NONDRAMA) {
				QJsonObject objBibleAudio;
				objBibleAudio["name"] = "FaithComesByHearing Non-Drama";
				objBibleAudio["url"] = QString(g_constrFCBHNonDramaURL).arg(strFCBHNonDrama);
				arrBibleAudioList.append(objBibleAudio);
			}
			if (flagsBAS & BAS_FCBH_DRAMA) {
				QJsonObject objBibleAudio;
				objBibleAudio["name"] = "FaithComesByHearing Drama";
				objBibleAudio["url"] = QString(g_constrFCBHDramaURL).arg(strFCBHDrama);
				arrBibleAudioList.append(objBibleAudio);
			}

			// Dan Wagner:
			QString strDWBookFolder = QString("%1_%2").arg(nBk, 2, 10, QChar('0')).arg(g_arrconstrDWBooks[nBk-1]);
			QString strDWBkChp;
			if (nBk == NUMBERS_BOOK_NUM) {
				strDWBkChp = QString("%1%2").arg(g_arrconstrDWBooks[nBk-1]).arg(nChp, 2, 10, QChar('0'));
			} else if ((nBk == PHILEMON_BOOK_NUM) ||
					   (nBk == _2JOHN_BOOK_NUM) ||
					   (nBk == _3JOHN_BOOK_NUM) ||
					   (nBk == JUDE_BOOK_NUM)) {
				strDWBkChp = g_arrconstrDWBooks[nBk-1];
			} else {
				strDWBkChp = QString("%1%2").arg(g_arrconstrDWBooks[nBk-1]).arg(nChp, 3, 10, QChar('0'));
			}
			QString strDanWagner = QString("%1/%2.mp3").arg(strDWBookFolder).arg(strDWBkChp);
			if (flagsBAS & BAS_DAN_WAGNER) {
				QJsonObject objBibleAudio;
				objBibleAudio["name"] = "Dan Wagner";
				objBibleAudio["url"] = QString(g_constrDWURL).arg(strDanWagner);
				arrBibleAudioList.append(objBibleAudio);
			}

			// Stephen Johnston:
			QString strSJBkChp = QString("%1%2%3.mp3").arg(nBk, 2, 10, QChar('0')).arg(g_arrconstrSJBooks[nBk-1]).arg(nChp, 3, 10, QChar('0'));
			if (flagsBAS & BAS_STEPHEN_JOHNSTON) {
				QJsonObject objBibleAudio;
				objBibleAudio["name"] = "Stephen Johnston";
				objBibleAudio["url"] = QString(g_constrSJURL).arg(strSJBkChp);
				arrBibleAudioList.append(objBibleAudio);
			}

			// Christophen Glyn:
			QString strCGTstFolder = QString("%1").arg((nTst == 1) ? "ot" : "nt");
			QString strCGBkChp = QString("%1_%2_%3").arg(nBkTst, 2, 10, QChar('0'))
													.arg(g_arrconstrCGBooks[nBk-1])
													.arg(nChp, (nBk == PSALMS_BOOK_NUM) ? 3 : 2, 10, QChar('0'));
			QString strChristopherGlyn = QString("%1/%2.mp3").arg(strCGTstFolder).arg(strCGBkChp);
			if (flagsBAS & BAS_CHRISTOPHER_GLYN) {
				QJsonObject objBibleAudio;
				objBibleAudio["name"] = "Christopher Glyn";
				objBibleAudio["url"] = QString(g_constrCGURL).arg(strChristopherGlyn);
				arrBibleAudioList.append(objBibleAudio);
			}

		}
	}

	return QJsonDocument(arrBibleAudioList).toJson(QJsonDocument::Compact);
}

// ============================================================================
