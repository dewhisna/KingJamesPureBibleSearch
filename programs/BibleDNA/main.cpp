//
// BibleDNA Study
//	to see if there are any DNA like patterns in the Hebrew
//	text as some, like Stan Tenen and Douglas Vogt, claim
//

#include "MainWindow.h"
#include "BibleText.h"
#include "HebrewLetters.h"

#include <QApplication>
#include <QMessageBox>

#include <iostream>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	CMainWindow w;

	w.show();

	if (MasoreticPentateuch::instance() == nullptr) {
		QMessageBox::critical(&w, QObject::tr("Bible Database", "main"),
				QObject::tr("Failed to read the Masoretic Pentateuch Bible Database!\nFrom: %1", "main").arg(MasoreticPentateuch::databasePath()));
		return -1;
	}

	const CBibleDatabase *pBible = MasoreticPentateuch::instance();
	std::cout << "Masoretic Pentateuch:" << std::endl;
	std::cout << "  Number of Books:    " << pBible->bibleEntry().m_nNumBk << std::endl;
	std::cout << "  Number of Chapters: " << pBible->bibleEntry().m_nNumChp << std::endl;
	std::cout << "  Number of Verses:   " << pBible->bibleEntry().m_nNumVrs << std::endl;
	std::cout << "  Number of Words:    " << pBible->bibleEntry().m_nNumWrd << std::endl;
	std::cout << "  Number of Letters:  " << pBible->bibleEntry().m_nNumLtr << std::endl;
	std::cout << std::endl;
	for (uint32_t nBk = 1; nBk <= pBible->bibleEntry().m_nNumBk; ++nBk) {
		const CBookEntry *pBook = pBible->bookEntry(nBk);
		if (pBook == nullptr) continue;
		std::cout << "Book #" << nBk << "  (" << pBook->m_strBkName.toStdString() << ")" << std::endl;
		std::cout << "  Number of Chapters: " << pBook->m_nNumChp << std::endl;
		std::cout << "  Number of Verses:   " << pBook->m_nNumVrs << std::endl;
		std::cout << "  Number of Words:    " << pBook->m_nNumWrd << std::endl;
		std::cout << "  Number of Letters:  " << pBook->m_nNumLtr << std::endl;
	}

	std::cout << "--------------------------------------" << std::endl;

//	for (int ndxPairs = 0; ndxPairs < CHebrewLetters::numberOfLetterPairs; ++ndxPairs) {
//		CHebrewLetters::TLetterPair pair = CHebrewLetters::base3LetterPairFromIndex(ndxPairs);
//		std::cout << "  : "
//				  << QString(CHebrewLetters::letterForIndex(pair.first)).toStdString()
//				  << " <-> "
//				  << QString(CHebrewLetters::letterForIndex(pair.second)).toStdString()
//				  << std::endl;
//	}
//	std::cout
//			<< CHebrewLetters::indexOfLetter(QString::fromUtf8("א").at(0)) << std::endl
//			<< CHebrewLetters::indexOfLetter(QString::fromUtf8("ב").at(0)) << std::endl
//			<< CHebrewLetters::indexOfLetter(QString::fromUtf8("ג").at(0)) << std::endl
//			<< CHebrewLetters::indexOfLetter(QString::fromUtf8("ד").at(0)) << std::endl
//			<< CHebrewLetters::indexOfLetter(QString::fromUtf8("ה").at(0)) << std::endl
//			<< CHebrewLetters::indexOfLetter(QString::fromUtf8("ו").at(0)) << std::endl
//			<< CHebrewLetters::indexOfLetter(QString::fromUtf8("ז").at(0)) << std::endl
//			<< CHebrewLetters::indexOfLetter(QString::fromUtf8("ח").at(0)) << std::endl
//			<< CHebrewLetters::indexOfLetter(QString::fromUtf8("ט").at(0)) << std::endl
//			<< CHebrewLetters::indexOfLetter(QString::fromUtf8("י").at(0)) << std::endl
//			<< CHebrewLetters::indexOfLetter(QString::fromUtf8("כ‬").at(0)) << std::endl
//			<< CHebrewLetters::indexOfLetter(QString::fromUtf8("ל").at(0)) << std::endl
//			<< CHebrewLetters::indexOfLetter(QString::fromUtf8("מ").at(0)) << std::endl
//			<< CHebrewLetters::indexOfLetter(QString::fromUtf8("נ").at(0)) << std::endl
//			<< CHebrewLetters::indexOfLetter(QString::fromUtf8("ס").at(0)) << std::endl
//			<< CHebrewLetters::indexOfLetter(QString::fromUtf8("ע").at(0)) << std::endl
//			<< CHebrewLetters::indexOfLetter(QString::fromUtf8("פ").at(0)) << std::endl
//			<< CHebrewLetters::indexOfLetter(QString::fromUtf8("צ").at(0)) << std::endl
//			<< CHebrewLetters::indexOfLetter(QString::fromUtf8("ק").at(0)) << std::endl
//			<< CHebrewLetters::indexOfLetter(QString::fromUtf8("ר‬").at(0)) << std::endl
//			<< CHebrewLetters::indexOfLetter(QString::fromUtf8("ש").at(0)) << std::endl
//			<< CHebrewLetters::indexOfLetter(QString::fromUtf8("ת").at(0)) << std::endl
//			<< CHebrewLetters::indexOfLetter(QString::fromUtf8("ך").at(0)) << std::endl
//			<< CHebrewLetters::indexOfLetter(QString::fromUtf8("ם").at(0)) << std::endl
//			<< CHebrewLetters::indexOfLetter(QString::fromUtf8("ן").at(0)) << std::endl
//			<< CHebrewLetters::indexOfLetter(QString::fromUtf8("ף").at(0)) << std::endl
//			<< CHebrewLetters::indexOfLetter(QString::fromUtf8("ץ").at(0)) << std::endl;
//
//	return 0;

	// Start with just Genesis and apply Stan Tenen's Theory:
	constexpr uint32_t conSequenceLength = 28;
	const CBookEntry *pGenesis = pBible->bookEntry(1);
	uint32_t nLetterNormalIndex = pBible->NormalizeIndexEx(CRelIndexEx(1, 1, 1, 1, 1));
	uint32_t nLtr = 1;
	while (nLtr <= pGenesis->m_nNumLtr) {
		QString strSequence;
		int arrPairCounts[CHebrewLetters::numberOfLetterPairs] = { };
		QString arrPairs[CHebrewLetters::numberOfLetterPairs] = { };
		int arrUnpairedCounts[CHebrewLetters::numberOfUnpairedLetters] = { };
		int arrLetterCounts[CHebrewLetters::HEBNDX_COUNT] = { };
		for (uint32_t ndxSeq = 0; ((ndxSeq < conSequenceLength) && (nLtr <= pGenesis->m_nNumLtr)); ++ndxSeq) {
			CRelIndexEx relLetterIndex = pBible->DenormalizeIndexEx(nLetterNormalIndex);
			const CConcordanceEntry *pConcordanceEntry = pBible->concordanceEntryForWordAtIndex(relLetterIndex);
			Q_ASSERT(pConcordanceEntry != nullptr);
			if (pConcordanceEntry != nullptr) {
				strSequence += pConcordanceEntry->letter(relLetterIndex.letter());
			}
			++nLtr;
			++nLetterNormalIndex;
		}
		std::cout << "Sequence: " << strSequence.toStdString() << std::endl;
		if (strSequence.size() < static_cast<int>(conSequenceLength)) {
			std::cout << "  ** Incomplete Sequence" << std::endl;
		}
		if (strSequence.isEmpty()) continue;
		for (int ndxSeq = 1; ndxSeq < strSequence.size()-1; ++ndxSeq) {
			CHebrewLetters::HEB_LTR_NDX ndxLetter = CHebrewLetters::indexOfLetter(strSequence.at(ndxSeq));
			if (ndxLetter == CHebrewLetters::HEBNDX_Unknown) {
				std::cout << "  ** Unknown Letter: " << QString(strSequence.at(ndxSeq)).toStdString() << std::endl;
			} else {
				++arrLetterCounts[ndxLetter];
				const CHebrewLetters::THebrewLetterBase3Pairing &base3Pair = CHebrewLetters::base3pairing(ndxLetter);
				if (base3Pair.m_bSelf) {
					++arrUnpairedCounts[base3Pair.m_nIndex];
				} else {
					++arrPairCounts[base3Pair.m_nIndex];
					arrPairs[base3Pair.m_nIndex] += strSequence.at(ndxSeq);
				}
			}
		}
		std::cout << "  Pairs:" << std::endl;
		for (int ndxPairs = 0; ndxPairs < CHebrewLetters::numberOfLetterPairs; ++ndxPairs) {
			Q_ASSERT(arrPairs[ndxPairs].size() == arrPairCounts[ndxPairs]);
			if (arrPairs[ndxPairs].isEmpty()) continue;
			std::cout << "    Pair: " << arrPairs[ndxPairs].toStdString() << std::endl;
		}
		std::cout << "  Self:" << std::endl;
		for (int ndxSelf = 0; ndxSelf < CHebrewLetters::numberOfUnpairedLetters; ++ndxSelf) {
			if (arrUnpairedCounts[ndxSelf] == 0) continue;
			std::cout << "    Self: " << QString(CHebrewLetters::letterForIndex(CHebrewLetters::base3LetterUnpairedFromIndex(ndxSelf))).toStdString()
					  << "  "
					  << arrUnpairedCounts[ndxSelf]
					  << std::endl;
		}
		std::cout << "  Start: " << QString(strSequence.at(0)).toStdString()
				  << "  End: " << QString(strSequence.at(strSequence.size()-1)).toStdString()
				  << std::endl;
	}

//	return a.exec();
}
