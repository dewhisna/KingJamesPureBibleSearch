/****************************************************************************
**
** Copyright (C) 2022-2025 Donna Whisnant, a.k.a. Dewtronics.
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
#include "../KJVCanOpener/ReadDB.h"
#include "../KJVCanOpener/VerseRichifier.h"
#include "../KJVCanOpener/ParseSymbols.h"
#include "../KJVCanOpener/Translator.h"
#include "../KJVCanOpener/PersistentSettings.h"
#ifdef USE_GEMATRIA
#include "../KJVCanOpener/Gematria.h"
#include "../KJVCanOpener/CSV.h"
#endif
// ----
#include "../KJVCanOpener/qwebchannel/webChannelBibleAudio.h"

#include <QCoreApplication>
#include <QLibraryInfo>
#include <QObject>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <QList>
#include <QtGlobal>
#if QT_VERSION < 0x050000
#include <QTextCodec>
#endif

#include <iostream>
#include <set>

#include "../KJVCanOpener/PathConsts.h"

#include "version.h"

namespace {
	//////////////////////////////////////////////////////////////////////
	// File-scoped constants
	//////////////////////////////////////////////////////////////////////

}	// namespace


// ============================================================================

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	a.setApplicationVersion(KJVDataDump_VERSION);

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
	bool bUnknownOption = false;
	bool bSkipColophons = false;
	bool bSkipSuperscriptions = false;
	bool bPrintReference = false;
	bool bPrintReferenceAbbrev = false;
	bool bPrintPilcrowMarkers = false;
	bool bOutputTemplates = false;
	bool bOutputVerseText = false;
	bool bOutputTransChangeAdded = false;
	bool bOutputVPL = false;
	bool bOutputWordsOnly = false;
	bool bOutputWordsAllUppercase = false;
	bool bOutputWordsAllLowercase = false;
	bool bOutputWebChannelBibleAudioURLs = false;
	bool bLookingForVersification = false;
#ifdef USE_GEMATRIA
	bool bOutputGematria = false;
#endif
	QString strV11n;

	for (int ndx = 1; ndx < argc; ++ndx) {
		QString strArg = QString::fromUtf8(argv[ndx]);
		if (!strArg.startsWith("-")) {
			if (bLookingForVersification) {
				strV11n = strArg;
				bLookingForVersification = false;
			} else {
				++nArgsFound;
				if (nArgsFound == 1) {
					nDescriptor = strArg.toInt();
				}
			}
		} else if (strArg.compare("-sc") == 0) {
			bSkipColophons = true;
		} else if (strArg.compare("-ss") == 0) {
			bSkipSuperscriptions = true;
		} else if (strArg.compare("-r") == 0) {
			bPrintReference = true;
		} else if (strArg.compare("-ra") == 0) {
			bPrintReference = true;
			bPrintReferenceAbbrev = true;
		} else if (strArg.compare("-p") == 0) {
			bPrintPilcrowMarkers = true;
		} else if (strArg.compare("-t") == 0) {
			bOutputTemplates = true;
		} else if (strArg.compare("-x") == 0) {
			bOutputVerseText = true;
		} else if (strArg.compare("-a") == 0) {
			bOutputVerseText = true;
			bOutputTransChangeAdded = true;
		} else if (strArg.compare("-vpl") == 0) {
			bOutputVPL = true;
			bPrintReference = true;
			bPrintPilcrowMarkers = true;
			bOutputVerseText = true;
		} else if (strArg.compare("-w") == 0) {
			bOutputWordsOnly = true;
			bOutputVerseText = true;
		} else if (strArg.compare("-u") == 0) {
			bOutputWordsAllUppercase = true;
		} else if (strArg.compare("-l") == 0) {
			bOutputWordsAllLowercase = true;
		} else if (strArg.compare("-wba") == 0) {
			bOutputWebChannelBibleAudioURLs = true;
		} else if (strArg.compare("-v11n") == 0) {
			bLookingForVersification = true;
#ifdef USE_GEMATRIA
		} else if (strArg.compare("-gematria") == 0) {
			bOutputGematria = true;
#endif
		} else {
			bUnknownOption = true;
		}
	}

	if (bLookingForVersification) bUnknownOption = true;	// Still looking for versification index
	if (bOutputWordsAllUppercase && bOutputWordsAllLowercase) bUnknownOption = true;	// Can't have both all upper and lower
	if (bOutputWordsOnly && bOutputTemplates) bUnknownOption = true;		// Can't have words only with templates
	if (bOutputWordsOnly && bPrintPilcrowMarkers) bUnknownOption = true;	// Can't have words only with pilcrows (note: this also excludes -vpl)
	if (bOutputTransChangeAdded && bOutputTemplates) bUnknownOption = true;	// Can't have transchange only text with templates
	if (bOutputTransChangeAdded && bPrintPilcrowMarkers) bUnknownOption = true;	// Can't have transchange only text with pilcrows (note: this also excludes -vpl)
	if (bOutputWordsOnly && bOutputTransChangeAdded) bUnknownOption = true;	// Can't have transchange only and words only as both are types of words-only modes

	if ((nArgsFound != 1) || (bUnknownOption)) {
		std::cerr << KJVDataDump_APPNAME << " Version " << KJVDataDump_VERSION_SEMVER << "\n\n";
		std::cerr << QString("Usage: %1 [options] <UUID-Index>\n\n").arg(argv[0]).toUtf8().data();
		std::cerr << QString("Reads the specified database and dumps relevant data for each verse\n\n").toUtf8().data();
		std::cerr << QString("Options are:\n").toUtf8().data();
		std::cerr << QString("  -sc =  Skip Colophons\n").toUtf8().data();
		std::cerr << QString("  -ss =  Skip Superscriptions\n").toUtf8().data();
		std::cerr << QString("  -r  =  Print Reference\n").toUtf8().data();
		std::cerr << QString("  -ra =  Print Abbreviated Reference (implies -r)\n").toUtf8().data();
		std::cerr << QString("  -p  =  Print Pilcrow Markers (when outputting text with -x)\n").toUtf8().data();
		std::cerr << QString("  -t  =  Print Verse Templates\n").toUtf8().data();
		std::cerr << QString("  -x  =  Print Verse Text\n").toUtf8().data();
		std::cerr << QString("  -a  =  Print Only TransChangeAdded Text (implies -x) (exclusive with -t, -p, -vpl, and -w)\n").toUtf8().data();
		std::cerr << QString("  -vpl = Print Verse Per Line format (implies -x, -r, and -p)\n").toUtf8().data();
		std::cerr << QString("  -w  =  Print Words only (implies -x) (exclusive with -t, -p, -vpl, and -a)\n").toUtf8().data();
		std::cerr << QString("  -u  =  Print Words in all uppercase (only with -a or -w) (exclusive with -l)\n").toUtf8().data();
		std::cerr << QString("  -l  =  Print Words in all uppercase (only with -a or -w) (exclusive with -u)\n").toUtf8().data();
		std::cerr << QString("  -wba = Print WebChannel Bible Audio URLs (supersedes other output modes)\n").toUtf8().data();
		std::cerr << QString("  -v11n <index> = Use versification <index> if the database supports it\n").toUtf8().data();
		std::cerr << QString("         (where <index> is one of the v11n indexes listed below\n").toUtf8().data();
#ifdef USE_GEMATRIA
		std::cerr << QString("  -gematria = Dump CSV output of all gematria counts (supersedes verse text output modes)\n").toUtf8().data();
#endif
		std::cerr << QString("\n").toUtf8().data();
		std::cerr << QString("UUID-Index:\n").toUtf8().data();
		for (unsigned int ndx = 0; ndx < bibleDescriptorCount(); ++ndx) {
			std::cerr << QString("    %1 = %2\n").arg(ndx).arg(bibleDescriptor(static_cast<BIBLE_DESCRIPTOR_ENUM>(ndx)).m_strDBDesc).toUtf8().data();
		}
		std::cerr << "\n";
		std::cerr << QString("Versification Index:\n").toUtf8().data();
		for (int ndx = 0; ndx < CBibleVersifications::count(); ++ndx) {
			std::cerr << QString("    %1 = %2\n").arg(ndx).arg(CBibleVersifications::name(static_cast<BIBLE_VERSIFICATION_TYPE_ENUM>(ndx))).toUtf8().data();
		}
		std::cerr << "\n";
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

	std::cerr << QString("Reading database: %1\n").arg(bblDescriptor.m_strDBName).toUtf8().data();

#ifdef USE_GEMATRIA
	// If outputting gematria, we must enable it before we
	//	read the database file or else the database won't
	//	have gematria values and we will crash below trying
	//	to use them:
	if (bOutputGematria) {
		TBibleDatabaseList::setUseGematria(true);
	}
#endif

	CReadDatabase rdbMain;
	if (!rdbMain.haveBibleDatabaseFiles(bblDescriptor)) {
		std::cerr << QString("\n*** ERROR: Unable to locate Bible Database Files!\n").toUtf8().data();
		return -2;
	}
	if (!rdbMain.ReadBibleDatabase(bblDescriptor, true)) {
		std::cerr << QString("\n*** ERROR: Failed to Read the Bible Database!\n").toUtf8().data();
		return -3;
	}

	// ------------------------------------------------------------------------

	CBibleDatabasePtr pBibleDatabase = TBibleDatabaseList::instance()->mainBibleDatabase();

	BIBLE_VERSIFICATION_TYPE_ENUM nV11nType = static_cast<BIBLE_VERSIFICATION_TYPE_ENUM>(strV11n.toUInt());
	if (!strV11n.isEmpty()) {
		if ((nV11nType < 0) || (nV11nType >= CBibleVersifications::count())) {
			std::cerr << "Unknown Versification Type Index specified\n";
			return -4;
		}

		if (!pBibleDatabase->hasVersificationType(nV11nType)) {
			std::cerr << "That Bible Database doesn't support Versification Index: " << nV11nType << "\n";
			return -5;
		}

		// Switch Versifications:
		TBibleDatabaseSettings settings = CPersistentSettings::instance()->bibleDatabaseSettings(pBibleDatabase->compatibilityUUID());
		settings.setVersification(nV11nType);
		CPersistentSettings::instance()->setBibleDatabaseSettings(pBibleDatabase->compatibilityUUID(), settings);
	}

	// ------------------------------------------------------------------------

	static QStringList lstVerseWords;

	class CMyVerseTextRichifierTags : public CVerseTextPlainRichifierTags
	{
	public:
		CMyVerseTextRichifierTags(bool bOutputText, bool bOutputTransChangeAdded, bool bOutputPilcrowMarkers,
									bool bOutputWordsOnly, bool bOutputWordsAllUppercase, bool bOutputWordsAllLowercase)
			:	m_bOutputText(bOutputText),
				m_bOutputTransChangeAdded(bOutputTransChangeAdded),
				m_bOutputWordsOnly(bOutputWordsOnly),
				m_bOutputWordsAllUppercase(bOutputWordsAllUppercase),
				m_bOutputWordsAllLowercase(bOutputWordsAllLowercase)
		{
			setShowPilcrowMarkers(bOutputPilcrowMarkers);
		}
	protected:
		virtual void wordCallback(const QString &strWord, VerseWordTypeFlags nWordTypes) const override
		{
			if (m_bOutputText && m_bOutputWordsOnly) {
				QString strOutputWord = StringParse::decompose(StringParse::deApostrophe(strWord, true), true);
				if (m_bOutputWordsAllUppercase) {
					lstVerseWords.append(strOutputWord.toUpper());
				} else if (m_bOutputWordsAllLowercase) {
					lstVerseWords.append(strOutputWord.toLower());
				} else {
					lstVerseWords.append(strOutputWord);
				}
			} else if ((m_bOutputText && !m_bOutputTransChangeAdded) ||
				(m_bOutputText && m_bOutputTransChangeAdded && (nWordTypes & VWT_TransChangeAdded))) {
				if (m_bOutputWordsAllUppercase) {
					lstVerseWords.append(strWord.toUpper());
				} else if (m_bOutputWordsAllLowercase) {
					lstVerseWords.append(strWord.toLower());
				} else {
					lstVerseWords.append(strWord);
				}
			}
		}
	private:
		bool m_bOutputText;
		bool m_bOutputTransChangeAdded;
		bool m_bOutputWordsOnly;
		bool m_bOutputWordsAllUppercase;
		bool m_bOutputWordsAllLowercase;
	} vtrt(bOutputVerseText, bOutputTransChangeAdded, bPrintPilcrowMarkers, bOutputWordsOnly, bOutputWordsAllUppercase, bOutputWordsAllLowercase);

#ifdef USE_GEMATRIA
	struct TGematriaVerseValues {
		uint32_t m_nNormalIndex = 0;
		CRelIndex m_nRelIndex;
		uint64_t m_arrnValues[GBTE_COUNT][GMTE_COUNT][GLTE_COUNT] = {};
		bool m_arrbUse[GBTE_COUNT][GMTE_COUNT][GLTE_COUNT] = {};			// Note: Inverted from logic in CGematriaCalc
	};
	typedef QList<TGematriaVerseValues> CGematriaVerseValueList;

	bool arrbGematriaColsToUse[GBTE_COUNT][GMTE_COUNT][GLTE_COUNT] = {};	// Note: Inverted from logic in CGematriaCalc
	CGematriaVerseValueList lstGematriaVerseValues;
#endif

	CRelIndex ndxCurrent = pBibleDatabase->calcRelIndex(CRelIndex(), CBibleDatabase::RIME_Start);
	CRelIndex ndxLastVPL;

	while (ndxCurrent.isSet()) {
		if ((bSkipColophons && ndxCurrent.isColophon()) ||
			(bSkipSuperscriptions && ndxCurrent.isSuperscription())) {
			// Must increment to next physical word index instead of using calculator movement
			//	in order to properly traverse colophons and superscriptions:
//			ndxCurrent = pBibleDatabase->calcRelIndex(ndxCurrent, CBibleDatabase::RIME_NextVerse);
			ndxCurrent = pBibleDatabase->calcRelIndex(ndxCurrent, CBibleDatabase::RIME_EndOfVerse);
			ndxCurrent = pBibleDatabase->DenormalizeIndex(pBibleDatabase->NormalizeIndex(ndxCurrent)+1);
			continue;
		}

		if (bOutputWebChannelBibleAudioURLs) {
			QString strJsonBA = CWebChannelBibleAudio::instance()->urlsForChapterAudio(pBibleDatabase, ndxCurrent);

			if (bPrintReference) {
				CRelIndex ndxChapter(ndxCurrent);
				ndxChapter.setVerse(0);
				ndxChapter.setWord(0);
				QString strRef = (bPrintReferenceAbbrev ?
										pBibleDatabase->PassageReferenceAbbrText(ndxChapter) :
										pBibleDatabase->PassageReferenceText(ndxChapter));
				std::cout << strRef.toUtf8().data() << " : ";
			}
			std::cout << strJsonBA.toUtf8().data() << std::endl;

			ndxCurrent = pBibleDatabase->calcRelIndex(ndxCurrent, CBibleDatabase::RIME_NextChapter);
			continue;
		} else {
#ifdef USE_GEMATRIA
			if (bOutputGematria) {
				CRefCountCalc refCalc(pBibleDatabase.data(), CRefCountCalc::RTE_WORD, ndxCurrent);
				unsigned int nCount = refCalc.ofVerse().second;
				uint32_t ndxNormal = pBibleDatabase->NormalizeIndex(ndxCurrent);
				TGematriaVerseValues gvv;
				gvv.m_nNormalIndex = ndxNormal;
				gvv.m_nRelIndex = ndxCurrent;
				while (nCount--) {
					for (uint32_t nBaseType = 0; nBaseType < GBTE_COUNT; ++nBaseType) {
						for (uint32_t nMathXform = 0; nMathXform < GMTE_COUNT; ++nMathXform) {
							for (uint32_t nLtrXform = 0; nLtrXform < GLTE_COUNT; ++nLtrXform) {
								const CConcordanceEntry *pConcordanceEntry = pBibleDatabase->concordanceEntryForWordAtIndex(ndxNormal);
								Q_ASSERT(pConcordanceEntry != nullptr);
								if (pConcordanceEntry != nullptr) {
									CGematriaIndex ndxGematria(nBaseType, nMathXform, nLtrXform);
									const CGematriaCalc &nWordGematria = pConcordanceEntry->gematria();
									gvv.m_arrnValues[nBaseType][nMathXform][nLtrXform] += nWordGematria.value(ndxGematria);
									gvv.m_arrbUse[nBaseType][nMathXform][nLtrXform] = gvv.m_arrbUse[nBaseType][nMathXform][nLtrXform] || !nWordGematria.skip(ndxGematria);
									arrbGematriaColsToUse[nBaseType][nMathXform][nLtrXform] = arrbGematriaColsToUse[nBaseType][nMathXform][nLtrXform] || gvv.m_arrbUse[nBaseType][nMathXform][nLtrXform];
								}
							}
						}
					}

					++ndxNormal;
				}
				lstGematriaVerseValues.push_back(gvv);
			} else {
#endif
				const CVerseEntry *pVerse = pBibleDatabase->verseEntry(ndxCurrent);
				if (pVerse) {
					lstVerseWords.clear();
					QString strParsedVerse = pBibleDatabase->richVerseText(ndxCurrent, vtrt);

					if ((!bOutputVerseText && bOutputTemplates) ||
						(bOutputVerseText && !lstVerseWords.isEmpty())) {
						QString strSpacer;
						if (bPrintReference) {
							if (!bOutputVPL) {
								CRelIndex ndxVerse(ndxCurrent);
								ndxVerse.setWord(0);
								QString strRef = (bPrintReferenceAbbrev ?
														pBibleDatabase->PassageReferenceAbbrText(ndxVerse) :
														pBibleDatabase->PassageReferenceText(ndxVerse));
								std::cout << strRef.toUtf8().data() << " : ";
								strSpacer.fill(QChar(' '), strRef.size() + 3);
							} else {
								if ((ndxLastVPL.book() != ndxCurrent.book()) || (ndxLastVPL.chapter() != ndxCurrent.chapter())) {
									// Print book/chapter headings:
									std::cout << "#" << pBibleDatabase->bookOSISAbbr(ndxCurrent).toUtf8().data() << "." << ndxCurrent.chapter() << std::endl;
								}
								// Print verse reference prefix:
								std::cout << "#" << pBibleDatabase->bookOSISAbbr(ndxCurrent).toUtf8().data()
											<< "." << ndxCurrent.chapter() << "." << ndxCurrent.verse() << " ";
								ndxLastVPL = ndxCurrent;
							}
						}
						if (bOutputTemplates) {
							std::cout << pVerse->m_strTemplate.toUtf8().data() << std::endl;
						}
						if (bOutputVerseText) {
							if (bOutputTemplates) std::cout << strSpacer.toUtf8().data();
							if (bOutputTransChangeAdded || bOutputWordsOnly) {
								std::cout << lstVerseWords.join(QChar(' ')).toUtf8().data();
							} else {
								std::cout << strParsedVerse.toUtf8().data();
							}
							std::cout << std::endl;
						}
					}
				}
#ifdef USE_GEMATRIA
			}
#endif
		}

		// Must increment to next physical word index instead of using calculator movement
		//	in order to properly traverse colophons and superscriptions:
//		ndxCurrent = pBibleDatabase->calcRelIndex(ndxCurrent, CBibleDatabase::RIME_NextVerse);
		ndxCurrent = pBibleDatabase->calcRelIndex(ndxCurrent, CBibleDatabase::RIME_EndOfVerse);
		ndxCurrent = pBibleDatabase->DenormalizeIndex(pBibleDatabase->NormalizeIndex(ndxCurrent)+1);
	}

#ifdef USE_GEMATRIA
	// Dump gematria calc results (done here to drop entire columns with no output):
	if (bOutputGematria) {
		QString strGematriaOutput;
		CCSVStream csv(&strGematriaOutput, QIODevice::WriteOnly);
		// Print CSV Headers:
		csv << QString("NormalIndex");
		csv << QString("RelIndex");
		csv << QString("Reference");
		for (uint32_t nBaseType = 0; nBaseType < GBTE_COUNT; ++nBaseType) {
			for (uint32_t nMathXform = 0; nMathXform < GMTE_COUNT; ++nMathXform) {
				for (uint32_t nLtrXform = 0; nLtrXform < GLTE_COUNT; ++nLtrXform) {
					if (arrbGematriaColsToUse[nBaseType][nMathXform][nLtrXform]) {
						CGematriaIndex ndxGematria(nBaseType, nMathXform, nLtrXform);
						csv << CGematriaNames::name(ndxGematria);
					}
				}
			}
		}
		csv.endLine();
		// Print Gematria results lines:
		for (auto const &gvv : lstGematriaVerseValues) {
			csv << QString("%1").arg(gvv.m_nNormalIndex);
			csv << gvv.m_nRelIndex.asAnchor();
			CRelIndex ndxRef = gvv.m_nRelIndex;
			if (!ndxRef.isColophon() && !ndxRef.isSuperscription()) ndxRef.setWord(0);		// Make refs print correctly without word index
			if (bPrintReferenceAbbrev) {
				csv << pBibleDatabase->PassageReferenceAbbrText(ndxRef, true);
			} else {
				csv << pBibleDatabase->PassageReferenceText(ndxRef, true);
			}
			for (uint32_t nBaseType = 0; nBaseType < GBTE_COUNT; ++nBaseType) {
				for (uint32_t nMathXform = 0; nMathXform < GMTE_COUNT; ++nMathXform) {
					for (uint32_t nLtrXform = 0; nLtrXform < GLTE_COUNT; ++nLtrXform) {
						if (arrbGematriaColsToUse[nBaseType][nMathXform][nLtrXform]) {
							CGematriaIndex ndxGematria(nBaseType, nMathXform, nLtrXform);
							csv << QString("%1").arg(gvv.m_arrnValues[nBaseType][nMathXform][nLtrXform]);
						}
					}
				}
			}
			csv.endLine();
		}
		std::cout << strGematriaOutput.toUtf8().data();
	}
#endif

	// ------------------------------------------------------------------------

//	return a.exec();
	return 0;
}

