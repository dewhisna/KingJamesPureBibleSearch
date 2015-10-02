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
#define PSALMS_BOOK_NUM 19

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

		bool bFCBHValid = (((nTst > 0) && (nTst <= 2)) && (nBkTst != 0) && ((nBk > 0) && (nBk <= NUM_BK)));
		if ((pBibleDatabase->compatibilityUUID().compare(bibleDescriptor(BDE_KJV).m_strUUID, Qt::CaseInsensitive) != 0) &&
			(pBibleDatabase->compatibilityUUID().compare(bibleDescriptor(BDE_KJVPCE).m_strUUID, Qt::CaseInsensitive) != 0) &&
			(pBibleDatabase->compatibilityUUID().compare(bibleDescriptor(BDE_KJVA).m_strUUID, Qt::CaseInsensitive) != 0) &&
			(pBibleDatabase->compatibilityUUID().compare(bibleDescriptor(BDE_KJV1611).m_strUUID, Qt::CaseInsensitive) != 0) &&
			(pBibleDatabase->compatibilityUUID().compare(bibleDescriptor(BDE_KJV1611A).m_strUUID, Qt::CaseInsensitive) != 0))
			bFCBHValid = false;

		if (bFCBHValid) {
			QString strFCBHBookName = QString("%1").arg(g_arrconstrFCBHBooks[nBk-1], -12, QChar('_'));
			QString strBkTst = QString("%1%2").arg((nTst == 1) ? "A" : "B").arg(nBkTst, 2, 10, QChar('0'));
			QString strChp = ((nBk == PSALMS_BOOK_NUM) ?
								QString("%1").arg(nChp, 3, 10, QChar('0')) :
								QString("_%1").arg(nChp, 2, 10, QChar('0')));
			QString strFCBHNonDrama = QString("%1__%2_%3ENGKJVC1DA.mp3").arg(strBkTst).arg(strChp).arg(strFCBHBookName);
			QString strFCBHDrama = QString("%1__%2_%3ENGKJVC2DA.mp3").arg(strBkTst).arg(strChp).arg(strFCBHBookName);
			if (flagsBAS & BAS_FCBH_NONDRAMA) {
				QJsonObject objBibleAudio;
				objBibleAudio["name"] = "Non-Drama";
				objBibleAudio["url"] = QString(g_constrFCBHNonDramaURL).arg(strFCBHNonDrama);
				arrBibleAudioList.append(objBibleAudio);
			}
			if (flagsBAS & BAS_FCBH_DRAMA) {
				QJsonObject objBibleAudio;
				objBibleAudio["name"] = "Drama";
				objBibleAudio["url"] = QString(g_constrFCBHDramaURL).arg(strFCBHDrama);
				arrBibleAudioList.append(objBibleAudio);
			}
		}
	}

	return QJsonDocument(arrBibleAudioList).toJson(QJsonDocument::Compact);
}

// ============================================================================
