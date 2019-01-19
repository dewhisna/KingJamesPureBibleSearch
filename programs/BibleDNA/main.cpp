

#include "MainWindow.h"
#include "BibleText.h"

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

	return a.exec();
}
