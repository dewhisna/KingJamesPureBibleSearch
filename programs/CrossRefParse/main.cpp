/****************************************************************************
**
** Copyright (C) 2024-2025 Donna Whisnant, a.k.a. Dewtronics.
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

#include "../KJVCanOpener/dbstruct.h"
#include "../KJVCanOpener/Translator.h"
#include "../KJVCanOpener/UserNotesDatabase.h"

#include <QCoreApplication>
#include <QLibraryInfo>
#include <QObject>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <QRegularExpression>
#include <QtGlobal>
#if QT_VERSION < 0x050000
#include <QTextCodec>
#endif

#include <iostream>
#include <set>

#include "../KJVCanOpener/PathConsts.h"

namespace {
	//////////////////////////////////////////////////////////////////////
	// File-scoped constants
	//////////////////////////////////////////////////////////////////////

	const unsigned int VERSION = 10000;		// Version 1.0.0

	// --------------------------------

	const QString constrKJNPrefix("kjn");
	const QString constrKJNNameSpaceURI("http://www.dewtronics.com/KingJamesPureBibleSearch/namespace");
	// ----
	const QString constrKJNDocumentTag("KJNDocument");
	const QString constrKJNDocumentTextTag("KJNDocumentText");
	const QString constrNotesTag("Notes");
	const QString constrNoteTag("Note");
	const QString constrHighlightingTag("Highlighting");
	const QString constrHighlighterDBTag("HighlighterDB");
	const QString constrHighlighterTagsTag("HighlighterTags");
	const QString constrPhraseTagTag("PhraseTag");
	const QString constrRelIndexTag("RelIndex");
	const QString constrCrossReferencesTag("CrossReferences");
	const QString constrCrossRefTag("CrossRef");
	const QString constrHighlighterDefinitionsTag("HighlighterDefinitions");
	const QString constrHighlighterDefTag("HighlighterDef");
	// ----
	const QString constrVersionAttr("Version");
	const QString constrRelIndexAttr("RelIndex");
	const QString constrCountAttr("Count");
	const QString constrValueAttr("Value");
	const QString constrSizeAttr("Size");
	const QString constrUUIDAttr("DatabaseUUID");
	const QString constrV11nAttr("Versification");
	const QString constrHighlighterNameAttr("HighlighterName");
	const QString constrColorAttr("Color");
	const QString constrBackgroundColorAttr("BackgroundColor");
	const QString constrEnabledAttr("Enabled");
	const QString constrVisibleAttr("Visible");
	const QString constrKeywordsAttr("Keywords");

}	// namespace

// ============================================================================

CRelIndex parseReference(const QString &strRef)
{
	CRelIndex ndxRetVal;

	QStringList lstRefParts = strRef.split(QChar('.'), Qt::SkipEmptyParts);
	if (lstRefParts.size() == 3) {
		for (int ndxBk = 0; ndxBk < g_arrBibleBooks.size(); ++ndxBk) {
			if (g_arrBibleBooks.at(ndxBk).m_lstOsisAbbr.at(0).compare(lstRefParts.at(0), Qt::CaseInsensitive) == 0) {
				ndxRetVal.setBook(ndxBk+1);
				ndxRetVal.setChapter(lstRefParts.at(1).toUInt());
				ndxRetVal.setVerse(lstRefParts.at(2).toUInt());
				break;
			}
		}
	}
	return ndxRetVal;
}

TPassageTag parsePassageReference(const QString &strRef)
{
	TPassageTag tagRetVal;

	if (strRef.contains(QChar('-'))) {						// Check for reference range
		QStringList lstRefs = strRef.split(QChar('-'), Qt::SkipEmptyParts);
		if (lstRefs.size() == 2) {							// Invalid without exactly two parts to the range
			CRelIndex ndxFirst = parseReference(lstRefs.at(0));
			CRelIndex ndxLast = parseReference(lstRefs.at(1));
			if (ndxFirst > ndxLast) std::swap(ndxFirst, ndxLast);

			tagRetVal = TPassageTag(ndxFirst, 1);			// Default to treating as a single reference for cases where range is invalid

			if ((ndxFirst.book() == ndxLast.book()) &&
				(ndxFirst.chapter() != 0) &&
				(ndxLast.chapter() != 0)) {					// For range, require them to be in the same book
				const CKJVBibleChapterVerseCounts *pKJVersification = CKJVBibleChapterVerseCounts::instance();
				const QStringList &lstChpVerseCounts = pKJVersification->at(ndxFirst.book()-1);
				if ((ndxFirst.chapter() <= static_cast<uint32_t>(lstChpVerseCounts.size())) &&
					(ndxLast.chapter() <= static_cast<uint32_t>(lstChpVerseCounts.size()))) {
					unsigned int nVerseCount = 1;
					if (ndxFirst.chapter() == ndxLast.chapter()) {
						nVerseCount = (ndxLast.verse() - ndxFirst.verse()) + 1;
					} else {
						nVerseCount += (lstChpVerseCounts.at(ndxFirst.chapter()-1).toUInt()) - ndxFirst.verse();
						for (uint32_t nChp = ndxFirst.chapter(); nChp < ndxLast.chapter(); ++nChp) {
							nVerseCount += lstChpVerseCounts.at(nChp).toUInt();		// Note: index is already shifted by 1 above
						}
						nVerseCount += ndxLast.verse();
					}
					tagRetVal.setVerseCount(nVerseCount);
				}
			}
		}
	} else {
		tagRetVal = TPassageTag(parseReference(strRef), 1);		// Single reference
	}

	// Note: TPassageTag sets the word to '1' and these need to be verse references,
	//	so change it back to '0':  TODO - Change TPassageTag to not set it to '1'??  What all does that break?
	tagRetVal.relIndex().setWord(0);

	return tagRetVal;
}

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	a.setApplicationVersion(QString("%1.%2.%3").arg(VERSION/10000).arg((VERSION/100)%100).arg(VERSION%100));

#if QT_VERSION < 0x050000
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
#endif

	g_strTranslationsPath = QFileInfo(QCoreApplication::applicationDirPath(), g_constrTranslationsPath).absoluteFilePath();
	g_strTranslationFilenamePrefix = QString::fromUtf8(g_constrTranslationFilenamePrefix);

	// Load translations and set main application based on our locale:
	CTranslatorList::instance()->setApplicationLanguage();

	int nArgsFound = 0;
	QString strCrossRefFilename;
	QString strKJNFilename;
	bool bForceOverwrite = false;		// Overwrite existing output file
	bool bUnknownOption = false;

	for (int ndx = 1; ndx < argc; ++ndx) {
		QString strArg = QString::fromUtf8(argv[ndx]);
		if (!strArg.startsWith("-")) {
			++nArgsFound;
			if (nArgsFound == 1) {
				strCrossRefFilename = strArg;
			} else if (nArgsFound == 2) {
				strKJNFilename = strArg;
			}
		} else if (strArg.compare("-f") == 0) {
			bForceOverwrite = true;
		} else {
			bUnknownOption = true;
		}
	}

	if ((nArgsFound != 2) || (bUnknownOption)) {
		std::cerr << QString("CrossRefParse Version %1\n\n").arg(a.applicationVersion()).toUtf8().data();
		std::cerr << QString("Usage: %1 [options] <CrossRefFile> <KJNOutputFile>\n\n").arg(argv[0]).toUtf8().data();
		std::cerr << QString("Reads the specified www.openbible.info cross-references file and generates\n").toUtf8().data();
		std::cerr << QString("    KJPBS Notes (KJN) database file\n\n").toUtf8().data();
		std::cerr << QString("Options are:\n").toUtf8().data();
		std::cerr << QString("  -f  =  Force overwrite KJN Output File\n").toUtf8().data();
		std::cerr << "\n";
		return -1;
	}

	// ------------------------------------------------------------------------

	QFile fiCrossRef(strCrossRefFilename);

	if (!fiCrossRef.open(QIODevice::ReadOnly)) {
		std::cerr << "*** Failed to open \"" << strCrossRefFilename.toUtf8().data() << "\" for reading.\n";
		return -2;
	}

	QFile fiKJN(strKJNFilename);

	if (fiKJN.exists() && !bForceOverwrite) {
		std::cerr << "*** File \"" << strKJNFilename.toUtf8().data() << "\" already exists.  Delete it to overwrite or pass -f option.\n";
		return -3;
	}
	if (!fiKJN.open(QIODevice::WriteOnly)) {
		std::cerr << "*** Failed to open \"" << strKJNFilename.toUtf8().data() << "\" for writing.\n";
		return -3;
	}

	// ------------------------------------------------------------------------

	TCrossReferenceMap mapCrossRefs;
	int nLineCount = 0;

	while (!fiCrossRef.atEnd()) {
		++nLineCount;
		QString strLine = QString::fromUtf8(fiCrossRef.readLine()).trimmed();
		if (strLine.startsWith("From Verse")) continue;		// Skip header

		QStringList lstRef = strLine.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
		if (lstRef.size() != 3) {
			std::cerr << "*** Unrecognized Line \"" << strLine.toUtf8().data() << "\" at line number: " << nLineCount << " -- Isn't 3 fields (skipping)\n";
			continue;
		}

		TPassageTag tagFrom = parsePassageReference(lstRef.at(0));
		TPassageTag tagTo = parsePassageReference(lstRef.at(1));

		if (!tagFrom.isSet()) {
			std::cerr << "*** Unable to convert 'From' reference on line: \"" << strLine.toUtf8().data() << "\" at line number: " << nLineCount << " (skipping)\n";
			continue;
		}
		if (!tagTo.isSet()) {
			std::cerr << "*** Unable to convert 'To' reference on line: \"" << strLine.toUtf8().data() << "\" at line number: " << nLineCount << " (skipping)\n";
			continue;
		}

		// TODO : Change/fix this when TCrossReferenceMap starts supporting TPassageTag and TPhraseTag:
		mapCrossRefs[tagFrom.relIndex()].insert(tagTo.relIndex());
	}

	QFile &outUND(fiKJN);		// TODO : Swap this to QtIOCompressor

	// Write our data in XML format:
	// Begin Document:
	outUND.write(QString("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n\n").toUtf8());
	outUND.write(QString("<%1:%2 xmlns:%1=\"%3\" "
						 "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
						 "xsi:schemaLocation=\"%3 kjnDocument.1.0.0.xsd\">\n")
					 .arg(constrKJNPrefix)
					 .arg(constrKJNDocumentTag)
					 .arg(constrKJNNameSpaceURI)
					 .toUtf8());

	// Begin Document Text:
	outUND.write(QString("\t<%1:%2 %3=\"%4\">\n").arg(constrKJNPrefix).arg(constrKJNDocumentTextTag)
					 .arg(constrVersionAttr).arg(KJN_FILE_VERSION).toUtf8());

	// CrossRefs:
	QString strVersification = QString();

	outUND.write(QString("\t\t<%1:%2 %3=\"%4\" %5=\"%6\">\n").arg(constrKJNPrefix).arg(constrCrossReferencesTag)
					 .arg(constrV11nAttr).arg(strVersification)
					 .arg(constrSizeAttr).arg(mapCrossRefs.size())
					 .toUtf8());
	for (TCrossReferenceMap::const_iterator itrCrossRef = mapCrossRefs.cbegin(); itrCrossRef != mapCrossRefs.cend(); ++itrCrossRef) {
		outUND.write(QString("\t\t\t<%1:%2 %3=\"%4\" %5=\"%6\">\n").arg(constrKJNPrefix).arg(constrCrossRefTag)
						 .arg(constrRelIndexAttr).arg((itrCrossRef->first).asAnchor())
						 .arg(constrSizeAttr).arg((itrCrossRef->second).size())
						 .toUtf8());
		for (TRelativeIndexSet::const_iterator itrTargetRefs = itrCrossRef->second.cbegin(); itrTargetRefs != itrCrossRef->second.cend(); ++itrTargetRefs) {
			outUND.write(QString("\t\t\t\t<%1:%2 %3=\"%4\" />\n").arg(constrKJNPrefix).arg(constrRelIndexTag)
							 .arg(constrValueAttr).arg((*itrTargetRefs).asAnchor())
							 .toUtf8());
		}
		outUND.write(QString("\t\t\t</%1:%2>\n").arg(constrKJNPrefix).arg(constrCrossRefTag).toUtf8());
	}
	outUND.write(QString("\t\t</%1:%2>\n").arg(constrKJNPrefix).arg(constrCrossReferencesTag).toUtf8());

	// End Document Text:
	outUND.write(QString("\t</%1:%2>\n").arg(constrKJNPrefix).arg(constrKJNDocumentTextTag).toUtf8());

	// End Document:
	outUND.write(QString("</%1:%2>\n").arg(constrKJNPrefix).arg(constrKJNDocumentTag).toUtf8());

	outUND.close();

	// ------------------------------------------------------------------------

//	return a.exec();
	return 0;
}

