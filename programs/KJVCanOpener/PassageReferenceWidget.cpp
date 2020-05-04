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

#include "PassageReferenceWidget.h"

#include "SearchCompleter.h"

#include <QRegExp>
#include <QStringList>

#ifdef QT_WIDGETS_LIB
#include <QColor>
#include <QKeyEvent>
#include <QMenu>
#endif

#include <assert.h>

#define PASSAGE_SOUNDEX_LENGTH 4
#define PASSAGE_SOUNDEX_MODE CSoundExSearchCompleterFilter::SEOME_ENHANCED

// ============================================================================

#ifdef QT_WIDGETS_LIB

CPassageReferenceWidget::CPassageReferenceWidget(QWidget *parent)
	:	QWidget(parent),
		m_pEditMenu(nullptr),
		m_pActionUndo(nullptr),
		m_pActionRedo(nullptr),
		m_pActionCut(nullptr),
		m_pActionCopy(nullptr),
		m_pActionPaste(nullptr),
		m_pActionDelete(nullptr),
		m_pActionSelectAll(nullptr)
{
	ui.setupUi(this);

	// ------------------------------------------------------------------------

	m_pEditMenu = new QMenu(tr("&Edit", "MainMenu"), ui.editPassageReference);
	m_pEditMenu->setStatusTip(tr("Passage Reference Editor Operations", "MainMenu"));

	m_pActionUndo = m_pEditMenu->addAction(tr("&Undo", "MainMenu"), ui.editPassageReference, SLOT(undo()), QKeySequence(Qt::CTRL + Qt::Key_Z));
	m_pActionUndo->setStatusTip(tr("Undo last operation to the Passage Reference Editor", "MainMenu"));
	m_pActionUndo->setEnabled(false);
//	connect(m_pActionUndo, SIGNAL(triggered()), ui.editPassageReference, SLOT(setFocus()));
	m_pActionRedo = m_pEditMenu->addAction(tr("&Redo", "MainMenu"), ui.editPassageReference, SLOT(redo()), QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Z));
	m_pActionRedo->setStatusTip(tr("Redo last operation on the Passage Reference Editor", "MainMenu"));
	m_pActionRedo->setEnabled(false);
//	connect(m_pActionRedo, SIGNAL(triggered()), ui.editPassageReference, SLOT(setFocus()));
	m_pEditMenu->addSeparator();
	m_pActionCut = m_pEditMenu->addAction(tr("Cu&t", "MainMenu"), ui.editPassageReference, SLOT(cut()), QKeySequence(Qt::CTRL + Qt::Key_X));
	m_pActionCut->setStatusTip(tr("Cut selected text from the Passage Reference Editor to the clipboard", "MainMenu"));
	m_pActionCut->setEnabled(false);
//	connect(m_pActionCut, SIGNAL(triggered()), ui.editPassageReference, SLOT(setFocus()));
	m_pActionCopy = m_pEditMenu->addAction(tr("&Copy", "MainMenu"), ui.editPassageReference, SLOT(copy()), QKeySequence(Qt::CTRL + Qt::Key_C));
	m_pActionCopy->setStatusTip(tr("Copy selected text from the Passage Reference Editor to the clipboard", "MainMenu"));
	m_pActionCopy->setEnabled(false);
//	connect(m_pActionCopy, SIGNAL(triggered()), ui.editPassageReference, SLOT(setFocus()));
	m_pActionPaste = m_pEditMenu->addAction(tr("&Paste", "MainMenu"), ui.editPassageReference, SLOT(paste()), QKeySequence(Qt::CTRL + Qt::Key_V));
	m_pActionPaste->setStatusTip(tr("Paste text on clipboard into the Passage Reference Editor", "MainMenu"));
	m_pActionPaste->setEnabled(true);
//	connect(m_pActionPaste, SIGNAL(triggered()), ui.editPassageReference, SLOT(setFocus()));
	m_pActionDelete = m_pEditMenu->addAction(tr("&Delete", "MainMenu"), ui.editPassageReference, SLOT(clear()), QKeySequence(Qt::Key_Delete));
	m_pActionDelete->setStatusTip(tr("Delete selected text from the Passage Reference Editor", "MainMenu"));
	m_pActionDelete->setEnabled(false);
//	connect(m_pActionDelete, SIGNAL(triggered()), ui.editPassageReference, SLOT(setFocus()));
	m_pEditMenu->addSeparator();
	m_pActionSelectAll = m_pEditMenu->addAction(tr("Select &All", "MainMenu"), ui.editPassageReference, SLOT(selectAll()), QKeySequence(Qt::CTRL + Qt::Key_A));
	m_pActionSelectAll->setStatusTip(tr("Select All Text in the Passage Reference Editor", "MainMenu"));
	m_pActionSelectAll->setEnabled(false);
//	connect(m_pActionSelectAll, SIGNAL(triggered()), ui.editPassageReference, SLOT(setFocus()));

	connect(ui.editPassageReference, SIGNAL(textChanged(const QString &)), this, SLOT(en_setMenuEnables(const QString &)));

	ui.editPassageReference->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui.editPassageReference, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(en_passageReferenceContextMenuRequested(const QPoint &)));

	ui.editPassageReference->installEventFilter(this);

	// ------------------------------------------------------------------------

	connect(ui.editPassageReference, SIGNAL(textChanged(const QString &)), this, SLOT(en_PassageReferenceChanged(const QString &)));
}

CPassageReferenceWidget::~CPassageReferenceWidget()
{

}

void CPassageReferenceWidget::initialize(CBibleDatabasePtr pBibleDatabase)
{
	if (!m_pRefResolver.isNull()) delete m_pRefResolver.data();
	m_pRefResolver = new CPassageReferenceResolver(pBibleDatabase, this);
}

bool CPassageReferenceWidget::hasFocusPassageReferenceEditor() const
{
	return ui.editPassageReference->hasFocus();
}

void CPassageReferenceWidget::clear()
{
	m_tagPhrase = TPhraseTag();
	ui.editPassageReference->clear();
}

void CPassageReferenceWidget::setPassageReference(const QString &strPassageReference)
{
	ui.editPassageReference->setText(strPassageReference);
}

bool CPassageReferenceWidget::eventFilter(QObject *pObject, QEvent *pEvent)
{
	assert(pEvent != nullptr);
	if ((pObject == ui.editPassageReference) && (pEvent->type() == QEvent::FocusIn)) emit activatedPassageReference();

	return QWidget::eventFilter(pObject, pEvent);
}

void CPassageReferenceWidget::focusInEvent(QFocusEvent *event)
{
	QWidget::focusInEvent(event);
	ui.editPassageReference->setFocus();
}

void CPassageReferenceWidget::keyPressEvent(QKeyEvent *event)
{
	if ((event) &&
		((event->key() == Qt::Key_Enter) ||
		 (event->key() == Qt::Key_Return))) {
		// Process enter/return so it won't propagate and "accept" the parent dialog:
		event->accept();
		emit enterPressed();
	} else {
		QWidget::keyPressEvent(event);
	}
}

void CPassageReferenceWidget::en_passageReferenceContextMenuRequested(const QPoint &pos)
{
	assert(m_pEditMenu != nullptr);
#ifndef USE_ASYNC_DIALOGS
	m_pEditMenu->exec(ui.editPassageReference->mapToGlobal(pos));
#else
	m_pEditMenu->popup(ui.editPassageReference->mapToGlobal(pos));
#endif
}

void CPassageReferenceWidget::en_setMenuEnables(const QString &strText)
{
	m_pActionUndo->setEnabled(ui.editPassageReference->isUndoAvailable());
	m_pActionRedo->setEnabled(ui.editPassageReference->isRedoAvailable());
	m_pActionCut->setEnabled(ui.editPassageReference->hasSelectedText());
	m_pActionCopy->setEnabled(ui.editPassageReference->hasSelectedText());
	m_pActionDelete->setEnabled(!strText.isEmpty());
	m_pActionSelectAll->setEnabled(!strText.isEmpty());
}

void CPassageReferenceWidget::en_PassageReferenceChanged(const QString &strText)
{
	assert(!m_pRefResolver.isNull());		// Run initialize first!
	if (m_pRefResolver.isNull()) return;

	m_tagPhrase = m_pRefResolver->resolve(strText);
	if ((m_tagPhrase.relIndex().isSet()) || (strText.trimmed().isEmpty())) {
		ui.editPassageReference->setStyleSheet(QString());
	} else {
		ui.editPassageReference->setStyleSheet(QString("QLineEdit { color:%1; }").arg(QColor("red").name()));
	}

	emit passageReferenceChanged(m_tagPhrase);
}

#endif	// QT_WIDGETS_LIB

// ============================================================================
// ============================================================================

CPassageReferenceResolver::CPassageReferenceResolver(CBibleDatabasePtr pBibleDatabase, QObject *pParent)
	:	QObject(pParent),
		m_pBibleDatabase(pBibleDatabase)
{
	assert(!pBibleDatabase.isNull());
	buildSoundExTables();
}

TPhraseTag CPassageReferenceResolver::resolve(const QString &strPassageReference) const
{
	assert(!m_pBibleDatabase.isNull());
	if (m_pBibleDatabase.isNull()) return TPhraseTag();

	//	From: http://stackoverflow.com/questions/9974012/php-preg-match-bible-scripture-format
	//
	//	RegExp: "\w+\s?(\d{1,3})?(:\d{1,3})?([-–]\d{1,3})?(,\s\d{1,3}[-–]\d{1,3})?"
	//	"
	//	\w         # Match a single character that is a “word character” (letters, digits, and underscores)
	//	   +          # Between one and unlimited times, as many times as possible, giving back as needed (greedy)
	//	\s         # Match a single character that is a “whitespace character” (spaces, tabs, and line breaks)
	//	   ?          # Between zero and one times, as many times as possible, giving back as needed (greedy)
	//	(          # Match the regular expression below and capture its match into backreference number 1
	//	   \d         # Match a single digit 0..9
	//	      {1,3}      # Between one and 3 times, as many times as possible, giving back as needed (greedy)
	//	)?         # Between zero and one times, as many times as possible, giving back as needed (greedy)
	//	(          # Match the regular expression below and capture its match into backreference number 2
	//	   :          # Match the character “:” literally
	//	   \d         # Match a single digit 0..9
	//	      {1,3}      # Between one and 3 times, as many times as possible, giving back as needed (greedy)
	//	)?         # Between zero and one times, as many times as possible, giving back as needed (greedy)
	//	(          # Match the regular expression below and capture its match into backreference number 3
	//	   [-–]       # Match a single character present in the list “-–”
	//	   \d         # Match a single digit 0..9
	//	      {1,3}      # Between one and 3 times, as many times as possible, giving back as needed (greedy)
	//	)?         # Between zero and one times, as many times as possible, giving back as needed (greedy)
	//	(          # Match the regular expression below and capture its match into backreference number 4
	//	   ,          # Match the character “,” literally
	//	   \s         # Match a single character that is a “whitespace character” (spaces, tabs, and line breaks)
	//	   \d         # Match a single digit 0..9
	//	      {1,3}      # Between one and 3 times, as many times as possible, giving back as needed (greedy)
	//	   [-–]       # Match a single character present in the list “-–”
	//	   \d         # Match a single digit 0..9
	//	      {1,3}      # Between one and 3 times, as many times as possible, giving back as needed (greedy)
	//	)?         # Between zero and one times, as many times as possible, giving back as needed (greedy)
	//	"

	//
	// Modified version written by me:
	//		Returns 8 submatch parts, plus the main expression match.  capturedTexts() count will be 9.
	//			Format is:  "1 John 2:3-4,5-6 [7]"
	//			0 :	"1 John 2:3-4,5-6 [7]"
	//			1 : "1"
	//			2 : "John"
	//			3 : "2"
	//			4 : "3"
	//			5 : "4"
	//			6 : "5"
	//			7 : "6"
	//			8 : "7"
	//
	//		- If there is no pre-book number, (1) will be empty.
	//		- If there is no ":" and verse number, then (4) will be empty, for example: "1 John 2-3,4-5 [6]"
	//			0 : "1 John 2-3,4-5"
	//			1 : "1"
	//			2 : "John"
	//			3 : "2"
	//			4 : ""
	//			5 : "3"
	//			6 : "4"
	//			7 : "5"
	//			8 : "6"
	//		- (5) is empty if no initial range is given
	//		- (6) and (7) will be empty if there is no ","
	//		- (7) is empty if no secondary range is given
	//		- (8) will be the Word Number, the value in the "[]" tags, if specified, or empty if not
	//

	QRegExp expReference("\\s*(\\d)?\\s*(\\w+)\\s*(\\d{1,3})?\\s*[:.]?\\s*(\\d{1,3})?\\s*[-–]?\\s*(\\d{1,3})?\\s*,?\\s*(\\d{1,3})?\\s*[-–]?\\s*(\\d{1,3})?\\s*\\[?\\s*(\\d{1,3})?\\s*\\]?", Qt::CaseInsensitive);

#define PARSENDX_ALL			0
#define PARSENDX_PREBOOK		1
#define PARSENDX_BOOK			2
#define PARSENDX_CHAPTER		3
#define PARSENDX_VERSE			4
#define PARSENDX_RANGEEND1		5
#define PARSENDX_RANGESTART2	6
#define PARSENDX_RANGEEND2		7
#define PARSENDX_WORD			8

	int nPos = expReference.indexIn(strPassageReference);
	QStringList lstMatches = expReference.capturedTexts();
	TPhraseTag tagResult;

#if 0
	qDebug("nPos=%d", nPos);
	for (int ndx=0; ndx<lstMatches.size(); ++ndx) {
		qDebug("Match %d : \"%s\"", ndx, lstMatches.at(ndx).toUtf8().data());
	}
#endif

	assert(lstMatches.size() == 9);

	if ((nPos != -1) && (lstMatches.size() == 9)) {
		CRelIndex ndxResolved;
		unsigned int nWordCount = 0;
		ndxResolved.setBook(resolveBook(lstMatches.at(PARSENDX_PREBOOK), lstMatches.at(PARSENDX_BOOK)));
		if (ndxResolved.book()) {
			const CBookEntry &book = *m_pBibleDatabase->bookEntry(ndxResolved.book());
			if (!lstMatches.at(PARSENDX_CHAPTER).isEmpty()) {
				// Resolve Chapter:
				ndxResolved.setChapter(lstMatches.at(PARSENDX_CHAPTER).toUInt());
				if ((ndxResolved.chapter() < 1) || (ndxResolved.chapter() > book.m_nNumChp)) ndxResolved.setChapter(0);
			} else {
				ndxResolved.setChapter(1);
			}
			if (ndxResolved.chapter()) {
				const CChapterEntry &chapter = *m_pBibleDatabase->chapterEntry(ndxResolved);
				if (!lstMatches.at(PARSENDX_VERSE).isEmpty()) {
					// Resolve Verse:
					ndxResolved.setVerse(lstMatches.at(PARSENDX_VERSE).toUInt());
					if ((ndxResolved.verse() < 1) || (ndxResolved.verse() > chapter.m_nNumVrs)) ndxResolved.setVerse(0);
				} else {
					ndxResolved.setVerse(1);
					ndxResolved.setWord(1);
					if (!lstMatches.at(PARSENDX_RANGEEND1).isEmpty()) {
						// Range of Chapters:
						uint32_t nLastChapter = lstMatches.at(PARSENDX_RANGEEND1).toUInt();
						if ((nLastChapter >= 1) && (nLastChapter <= book.m_nNumChp) && (nLastChapter >= ndxResolved.chapter())) {
							CRelIndex ndxLast(ndxResolved.book(), nLastChapter, 1, 1);
							const CChapterEntry &chapterLast = *m_pBibleDatabase->chapterEntry(ndxLast);
							ndxLast.setVerse(chapterLast.m_nNumVrs);
							const CVerseEntry &verseLast = *m_pBibleDatabase->verseEntry(ndxLast);
							ndxLast.setWord(verseLast.m_nNumWrd);
							nWordCount = m_pBibleDatabase->NormalizeIndex(ndxLast) - m_pBibleDatabase->NormalizeIndex(ndxResolved) + 1;
						}
					}
				}
				if (ndxResolved.verse() && (!lstMatches.at(PARSENDX_VERSE).isEmpty())) {
					const CVerseEntry &verse = *m_pBibleDatabase->verseEntry(ndxResolved);
					if (!lstMatches.at(PARSENDX_WORD).isEmpty()) {
						ndxResolved.setWord(lstMatches.at(PARSENDX_WORD).toUInt());
						if ((ndxResolved.word() < 1) || (ndxResolved.word() > verse.m_nNumWrd)) ndxResolved.setWord(0);
						nWordCount = 1;
					} else {
						ndxResolved.setWord(1);
					}
					if (!lstMatches.at(PARSENDX_RANGEEND1).isEmpty() && ndxResolved.word()) {
						// Range of Verses:
						uint32_t nLastVerse = lstMatches.at(PARSENDX_RANGEEND1).toUInt();
						if ((nLastVerse >= 1) && (nLastVerse <= chapter.m_nNumVrs) && (nLastVerse >= ndxResolved.verse())) {
							CRelIndex ndxLast = ndxResolved;
							ndxLast.setVerse(nLastVerse);
							const CVerseEntry &verseLast = *m_pBibleDatabase->verseEntry(ndxLast);
							ndxLast.setWord(verseLast.m_nNumWrd);
							nWordCount = m_pBibleDatabase->NormalizeIndex(ndxLast) - m_pBibleDatabase->NormalizeIndex(ndxResolved) + 1;
						}
					}
				}
			}
		}

		if ((ndxResolved.book() != 0) &&
			(ndxResolved.chapter() != 0) &&
			(ndxResolved.verse() != 0) &&
			(ndxResolved.word() != 0)) {
			tagResult = TPhraseTag(ndxResolved, nWordCount);
		}
	}

	return tagResult;
}

// ============================================================================

void CPassageReferenceResolver::buildSoundExTables()
{
	assert(!m_pBibleDatabase.isNull());
	m_lstBookSoundEx.clear();
	m_lstBookSoundEx.reserve(m_pBibleDatabase->bibleEntry().m_nNumBk);

	for (unsigned int nBk = 1; nBk <= m_pBibleDatabase->bibleEntry().m_nNumBk; ++nBk) {
		const CBookEntry &book = *m_pBibleDatabase->bookEntry(nBk);
		QStringList lstSoundEx;
		for (int nAbbr = 0; nAbbr <= book.m_lstBkAbbr.size(); ++nAbbr) {		// Index 0 will be used for entire book name others will index into abbr array
			QString strBookName = ((nAbbr == 0) ? book.m_strBkName.toLower() : book.m_lstBkAbbr.at(nAbbr-1).toLower());
			strBookName.replace(QRegExp("\\s"), QString());
			QRegExp regexpPrefix("^(\\d*)?");
			int nPosPrefix = regexpPrefix.indexIn(strBookName);
			assert(nPosPrefix != -1);
			assert(regexpPrefix.capturedTexts().size() == 2);
			QString strPrefix = regexpPrefix.capturedTexts().at(1);
			lstSoundEx.append(strPrefix + CSoundExSearchCompleterFilter::soundEx(strBookName,
																			CSoundExSearchCompleterFilter::languageValue(m_pBibleDatabase->language()),
																			PASSAGE_SOUNDEX_LENGTH,
																			PASSAGE_SOUNDEX_MODE));
		}
		m_lstBookSoundEx.append(lstSoundEx);
	}
}

uint32_t CPassageReferenceResolver::resolveBook(const QString &strPreBook, const QString &strBook) const
{
	assert(!m_pBibleDatabase.isNull());
	QString strBookName = strPreBook.toLower() + strBook.toLower();
	QString strSoundEx = strPreBook + CSoundExSearchCompleterFilter::soundEx(strBookName,
																			 CSoundExSearchCompleterFilter::languageValue(m_pBibleDatabase->language()),
																			 PASSAGE_SOUNDEX_LENGTH,
																			 PASSAGE_SOUNDEX_MODE);
	uint32_t nResolvedBook = 0;
	QList<uint32_t> lstResolvedBooks;
	for (unsigned int nBk = 1; nBk <= m_pBibleDatabase->bibleEntry().m_nNumBk; ++nBk) {
		const CBookEntry &book = *m_pBibleDatabase->bookEntry(nBk);
		for (int nAbbr = 0; nAbbr <= book.m_lstBkAbbr.size(); ++nAbbr) {		// Index 0 will be used for entire book name others will index into abbr array
			QString strBibleBookName = ((nAbbr == 0) ? book.m_strBkName.toLower() : book.m_lstBkAbbr.at(nAbbr-1).toLower());
			strBibleBookName.replace(QRegExp("\\s"), QString());
			if ((strSoundEx.compare(m_lstBookSoundEx.at(nBk-1).at(nAbbr)) == 0) || (strBibleBookName.startsWith(strBookName))) {
				lstResolvedBooks.append(nBk);
				break;			// Only need to add the book once
			}
		}
	}
	if (lstResolvedBooks.size() > 1) {
		bool bIsUnique = true;
		bool bExactMatch = false;
		for (int ndx = 0; ndx < lstResolvedBooks.size(); ++ndx) {
			const CBookEntry &book = *m_pBibleDatabase->bookEntry(lstResolvedBooks.at(ndx));
			for (int nAbbr = 0; nAbbr <= book.m_lstBkAbbr.size(); ++nAbbr) {		// Index 0 will be used for entire book name others will index into abbr array
				QString strBibleBookName = ((nAbbr == 0) ? book.m_strBkName.toLower() : book.m_lstBkAbbr.at(nAbbr-1).toLower());
				strBibleBookName.replace(QRegExp("\\s"), QString());
				if (strBibleBookName.startsWith(strBookName)) {
					if ((nResolvedBook == 0) || (strBibleBookName.compare(strBookName) == 0)) {
						nResolvedBook = lstResolvedBooks.at(ndx);
						if (strBibleBookName.compare(strBookName) == 0) {
							bExactMatch = true;
							break;					// Stop if we get an exact match...
						}
					} else {
						bIsUnique = false;
					}
				}
			}
		}
		if ((!bIsUnique) && (!bExactMatch)) nResolvedBook = 0;
	} else if (lstResolvedBooks.size() == 1) {
		nResolvedBook = lstResolvedBooks.at(0);
	}

	return nResolvedBook;
}

// ============================================================================
