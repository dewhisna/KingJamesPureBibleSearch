/****************************************************************************
**
** Copyright (C) 2016-2025 Donna Whisnant, a.k.a. Dewtronics.
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
#include "../KJVCanOpener/dbDescriptors.h"
#include "../KJVCanOpener/ReadDBEx.h"
#include "../KJVCanOpener/VerseRichifier.h"
#include "../KJVCanOpener/Translator.h"
#include "../KJVCanOpener/PassageReferenceResolver.h"

#include <QCoreApplication>
#include <QLibraryInfo>
#include <QObject>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <QtGlobal>
#if QT_VERSION < 0x050000
#include <QTextCodec>
#endif

#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QByteArray>

#include <iostream>
#include <set>

#include "../KJVCanOpener/PathConsts.h"

namespace {
	//////////////////////////////////////////////////////////////////////
	// File-scoped constants
	//////////////////////////////////////////////////////////////////////

	const unsigned int VERSION = 20000;		// Version 2.0.0

}	// namespace

// ============================================================================

TBibleDatabaseSettings::HideHyphensOptionFlags g_nHideHyphenOptions = TBibleDatabaseSettings::HHO_None;
bool g_bIncludeRef = false;
bool g_bUseAbbrRef = false;
bool g_bUseHTML = false;
bool g_bNoColorizeWordsOfJesus = false;
bool g_bHideTransChange = false;
bool g_bTransChangeAsBrackets = false;
bool g_bHidePilcrows = false;
bool g_bHidePs119 = false;
bool g_bAddNewline = false;
bool g_bQuiet = false;

// ============================================================================

QString doLookup(const QString &strReference)
{
	QString strResult;

	if (!g_bQuiet) {
		std::cerr << QString("Resolving: \"%1\"\n").arg(strReference).toUtf8().data();
	}

	CBibleDatabasePtr pBibleDatabase = TBibleDatabaseList::instance()->mainBibleDatabase();

	TPhraseTag tagRefIndex;

	if (strReference.startsWith(QChar('#'))) {
		tagRefIndex = TPhraseTag(CRelIndex(strReference.mid(1)));
	} else {
		CPassageReferenceResolver resolver(pBibleDatabase);
		tagRefIndex = resolver.resolve(strReference);
	}

	if (!tagRefIndex.isSet() || !pBibleDatabase->completelyContains(tagRefIndex)) {
		std::cerr << QString("\n*** ERROR: Bible Database doesn't contain reference \"%1\"!\n").arg(strReference).toUtf8().data();
		return QString();
	}

	CVerseTextRichifierTags richifierTags = (g_bUseHTML ? CVerseTextRichifierTags() : CVerseTextPlainRichifierTags());
	if (g_bNoColorizeWordsOfJesus) richifierTags.setWordsOfJesusTags(QString(), QString());
	richifierTags.setShowPilcrowMarkers(!g_bHidePilcrows);
	richifierTags.setAddRichPs119HebrewPrefix(!g_bHidePs119);
	if (g_bHideTransChange) {
		richifierTags.setTransChangeAddedTags(QString(), QString());
	} else if (g_bTransChangeAsBrackets) {
		richifierTags.setTransChangeAddedTags("[", "]");
	}

	TBibleDatabaseSettings bdbSettings = pBibleDatabase->settings();
	bdbSettings.setHideHyphens(g_nHideHyphenOptions);
	pBibleDatabase->setSettings(bdbSettings);

	if (g_bIncludeRef) {
		if (g_bUseHTML) strResult += "<b>";

		CRelIndex relIndex = tagRefIndex.relIndex();
		if (!relIndex.isColophon() && !relIndex.isSuperscription()) relIndex.setWord(0);
		if (g_bUseAbbrRef) {
			strResult += pBibleDatabase->PassageReferenceAbbrText(relIndex, true);
		} else {
			strResult += pBibleDatabase->PassageReferenceText(relIndex, true);
		}

		if (!g_bUseHTML) {
			strResult += ": ";
		} else {
			strResult += "</b>";
			strResult += "&nbsp;";
		}
	}
	CRelIndex relIndexWord1 = tagRefIndex.relIndex();
	relIndexWord1.setWord(1);
	const CVerseEntry *pVerse = pBibleDatabase->verseEntry(relIndexWord1);
	if (pVerse) {
		strResult += CVerseTextRichifier::parse(relIndexWord1, pBibleDatabase.data(), pVerse, richifierTags);
	} else {
		if (!g_bUseHTML) {
			strResult += "<NULL>";
		} else {
			strResult += "&lt;NULL&gt;";
		}
	}
	if (g_bAddNewline) {
		strResult += "\n";
	}

	return strResult;
}

// ============================================================================

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

	int nDescriptor = -1;
	int nArgsFound = 0;
	TBibleDescriptor bblDescriptor;
	QString strReference;
	bool bUnknownOption = false;
	CReadDatabaseEx::DB_OVERRIDE_ENUM nDBOE = CReadDatabaseEx::DBOE_None;

	for (int ndx = 1; ndx < argc; ++ndx) {
		QString strArg = QString::fromUtf8(argv[ndx]);
		if (!strArg.startsWith("-")) {
			++nArgsFound;
			if (nArgsFound == 1) {
				nDescriptor = strArg.toInt();
			} else if (nArgsFound == 2) {
				strReference = strArg;
			}
		} else if (strArg.compare("-h0") == 0) {
			g_nHideHyphenOptions = TBibleDatabaseSettings::HHO_None;
		} else if (strArg.compare("-h1") == 0) {
			g_nHideHyphenOptions = TBibleDatabaseSettings::HHO_ProperWords;
		} else if (strArg.compare("-h2") == 0) {
			g_nHideHyphenOptions = TBibleDatabaseSettings::HHO_OrdinaryWords;
		} else if (strArg.compare("-h3") == 0) {
			g_nHideHyphenOptions = TBibleDatabaseSettings::HideHyphensOptionFlags(
									TBibleDatabaseSettings::HHO_ProperWords |
									TBibleDatabaseSettings::HHO_OrdinaryWords);
		} else if (strArg.compare("-r") == 0) {
			g_bIncludeRef = true;
		} else if (strArg.compare("-a") == 0) {
			g_bUseAbbrRef = true;
		} else if (strArg.compare("-m") == 0) {
			g_bUseHTML = true;
		} else if (strArg.compare("-j") == 0) {
			g_bNoColorizeWordsOfJesus = true;
		} else if (strArg.compare("-t") == 0) {
			g_bHideTransChange = true;
		} else if (strArg.compare("-b") == 0) {
			g_bTransChangeAsBrackets = true;
		} else if (strArg.compare("-p") == 0) {
			g_bHidePilcrows = true;
		} else if (strArg.compare("-119") == 0) {
			g_bHidePs119 = true;
		} else if (strArg.compare("-n") == 0) {
			g_bAddNewline = true;
		} else if (strArg.compare("-q") == 0) {
			g_bQuiet = true;
		} else if (strArg.startsWith("-dbo")) {
			nDBOE = static_cast<CReadDatabaseEx::DB_OVERRIDE_ENUM>(strArg.mid(4).toInt());
			if ((nDBOE < 0) || (nDBOE >= CReadDatabaseEx::DBOE_COUNT)) bUnknownOption = true;
		} else {
			bUnknownOption = true;
		}
	}

	if ((nArgsFound != 2) || (bUnknownOption)) {
		std::cerr << QString("KJVLookup Version %1\n\n").arg(a.applicationVersion()).toUtf8().data();
		std::cerr << QString("Usage: %1 [options] <UUID-Index> <Reference>\n\n").arg(argv[0]).toUtf8().data();
		std::cerr << QString("Reads the specified database, looks up the specified reference,\n").toUtf8().data();
		std::cerr << QString("    and outputs the corresponding verse text\n\n").toUtf8().data();
		std::cerr << QString("Options are:\n").toUtf8().data();
		std::cerr << QString("  -h0 =  Hyphens Hide None (i.e. display all, this is default)\n").toUtf8().data();
		std::cerr << QString("  -h1 =  Hyphens Hide in Proper Words\n").toUtf8().data();
		std::cerr << QString("  -h2 =  Hyphens Hide in Ordinary Words\n").toUtf8().data();
		std::cerr << QString("  -h3 =  Hyphens Hide in Both\n").toUtf8().data();
		std::cerr << QString("  -r  =  Include reference text\n").toUtf8().data();
		std::cerr << QString("  -a  =  Use abbreviated reference (only with -r mode)\n").toUtf8().data();
		std::cerr << QString("  -m  =  Use HTML Markup (default is plain text)\n").toUtf8().data();
		std::cerr << QString("  -j  =  Don't Markup Words of Jesus (only for -m mode)\n").toUtf8().data();
		std::cerr << QString("  -t  =  Hide Translation Change Markup\n").toUtf8().data();
		std::cerr << QString("  -b  =  Use Brackets for TransChange (for -m mode)\n").toUtf8().data();
		std::cerr << QString("  -p  =  Hide Pilcrows\n").toUtf8().data();
		std::cerr << QString("  -119=  Hide Psalm 119 Hebrew Prefixes\n").toUtf8().data();
		std::cerr << QString("  -n  =  Add Trailing Newline\n").toUtf8().data();
		std::cerr << QString("  -q  =  Quiet mode (suppress unnecessary output)\n").toUtf8().data();
		std::cerr << QString("  -dbo<n> = Database Override Option\n").toUtf8().data();
		std::cerr << QString("          where <n> is one of the following:\n").toUtf8().data();
		for (int ndx = 0; ndx < CReadDatabaseEx::DBOE_COUNT; ++ndx) {
			std::cerr << QString("            %1 : %2%3\n")
							.arg(ndx)
							.arg(CReadDatabaseEx::dboeDescription(static_cast<CReadDatabaseEx::DB_OVERRIDE_ENUM>(ndx)))
							.arg((ndx == CReadDatabaseEx::DBOE_None) ? " (default)" : "").toUtf8().data();
		}
		std::cerr << QString("\n").toUtf8().data();
		std::cerr << QString("UUID-Index:\n").toUtf8().data();
		for (unsigned int ndx = 0; ndx < bibleDescriptorCount(); ++ndx) {
			std::cerr << QString("    %1 = %2\n").arg(ndx).arg(bibleDescriptor(static_cast<BIBLE_DESCRIPTOR_ENUM>(ndx)).m_strDBDesc).toUtf8().data();
		}
		std::cerr << "\n";
		std::cerr << QString("Reference : Can be #nnnn anchor format or Phrase Ref to parse\n").toUtf8().data();
		std::cerr << QString("         Examples:  #721620992\n").toUtf8().data();
		std::cerr << QString("                    \"jn 3:16\"\n\n").toUtf8().data();
		std::cerr << QString("   OR, use ^pppp to listen on port 'pppp' as a server\n").toUtf8().data();
		std::cerr << QString("         Example:   ^12345 (listen on port 12345)\n\n").toUtf8().data();
		return -1;
	}

	// TODO : Add support for raw-UUID as well as indexes

	if ((nDescriptor >= 0) && (static_cast<unsigned int>(nDescriptor) < bibleDescriptorCount())) {
		bblDescriptor = TBibleDatabaseList::availableBibleDatabaseDescriptor(bibleDescriptor(static_cast<BIBLE_DESCRIPTOR_ENUM>(nDescriptor)).m_strUUID);
	} else {
		std::cerr << "Unknown UUID-Index\n";
		return -1;
	}

	// ------------------------------------------------------------------------

	if (!g_bQuiet) {
		std::cerr << QString("Reading database: %1\n").arg(bblDescriptor.m_strDBName).toUtf8().data();
		std::cerr << QString("Database Override Option: %1\n").arg(CReadDatabaseEx::dboeDescription(nDBOE)).toUtf8().data();
	}

	CReadDatabaseEx rdbMain(nDBOE);
	if (!rdbMain.haveBibleDatabaseFiles(bblDescriptor)) {
		std::cerr << QString("\n*** ERROR: Unable to locate Bible Database Files!\n").toUtf8().data();
		return -2;
	}
	if (!rdbMain.ReadBibleDatabase(bblDescriptor, true)) {
		std::cerr << QString("\n*** ERROR: Failed to Read the Bible Database!\n").toUtf8().data();
		return -3;
	}

	// ------------------------------------------------------------------------

	// Handle non-server case:
	if (!strReference.startsWith(QChar('^'))) {
		std::cout << doLookup(strReference).toUtf8().data();
		return 0;
	}

	// ------------------------------------------------------------------------

	// Run lookup server here:
	QTcpServer myServer;

	if (!g_bQuiet) {
		std::cerr << QString("Starting KJVLookup server on port: %1\n").arg(strReference.mid(1)).toUtf8().data();
	}

	// Force newline in server mode so that client can do a readLine:
	g_bAddNewline = true;

	if (!myServer.listen(QHostAddress::LocalHost, strReference.mid(1).toUInt())) {
		std::cerr << QString("*** ERROR: Failed to start KJVLookup on port %1\n%2\n").arg(strReference.mid(1)).arg(myServer.errorString()).toUtf8().data();
		return -4;
	}

	if (!myServer.waitForNewConnection(-1)) {
		std::cerr << QString("*** ERROR: Failed to receive client connection\n%1\n").arg(myServer.errorString()).toUtf8().data();
		return -5;
	}

	if (!g_bQuiet) {
		std::cerr << "Client connected... waiting for read...\n";
	}

	QTcpSocket *pClient = myServer.nextPendingConnection();
	Q_ASSERT(pClient);
	if (pClient) {
		while (pClient->waitForReadyRead(30000)) {		// Must see commands in 30 second window
			QByteArray baCommand = pClient->readLine();
			QString strCommand(baCommand);
			if (strCommand.startsWith(":lookup:")) {
				pClient->write(doLookup(strCommand.mid(8).trimmed()).toUtf8());
				pClient->waitForBytesWritten();		// ?? needed ??
			} else if (strCommand.startsWith(":quit:")) {
				break;				// Drop out of loop and drop connection
			} else {
				std::cerr << QString("Unknown command received: \"%1\"\n").arg(strCommand).toUtf8().data();
			}
		}
	}

//	return a.exec();
	return 0;
}

