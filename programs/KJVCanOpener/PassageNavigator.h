/****************************************************************************
**
** Copyright (C) 2012-2025 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef PASSAGE_NAVIGATOR_H
#define PASSAGE_NAVIGATOR_H

#include "dbstruct.h"
#include "ScriptureEdit.h"

#include <QFlags>
#include <QWidget>

// ============================================================================

#include "ui_PassageNavigator.h"

class CPassageNavigator : public QWidget
{
	Q_OBJECT

public:
	// Specific RefType Selected:
	enum NAVIGATOR_REF_TYPE_ENUM {
		NRTE_WORD = 0,
		NRTE_VERSE = 1,
		NRTE_CHAPTER = 2,
		NRTE_BOOK = 3
	};

	// Allowed RefType Selections:
	enum NavigatorRefTypeOptions {
		NRTO_Default = 0x0,							// Default Options (all)
		NRTO_Word = 0x1,							// Allow Word Reference
		NRTO_Verse = 0x2,							// Allow Verse Reference
		NRTO_Chapter = 0x4,							// Allow Chapter Reference
		NRTO_Book = 0x8								// Allow Book Reference
	};
	Q_DECLARE_FLAGS(NavigatorRefTypeOptionFlags, NavigatorRefTypeOptions)


	CPassageNavigator(CBibleDatabasePtr pBibleDatabase, QWidget *parent = nullptr, NavigatorRefTypeOptionFlags flagsRefTypes = NRTO_Default, NAVIGATOR_REF_TYPE_ENUM nRefType = NRTE_WORD);
	virtual ~CPassageNavigator();

	TPhraseTag passage() const;
	void setPassage(const TPhraseTag &tag);
	TPhraseTag startRef() const { return m_tagStartRef; }

	NAVIGATOR_REF_TYPE_ENUM refType() const { return m_nRefType; }
	void setRefType(NAVIGATOR_REF_TYPE_ENUM nRefType);

	bool isReversed() const;
	bool isRelative() const { return m_tagStartRef.relIndex().isSet(); }
	bool isAbsolute() const { return (!m_tagStartRef.relIndex().isSet()); }

public slots:
	void startRelativeMode(TPhraseTag tagStart = TPhraseTag(CRelIndex(), 1), TPhraseTag tagPassage = TPhraseTag(CRelIndex(), 1));
	void startRelativeMode(TPhraseTag tagStart, bool bReverse, TPhraseTag tagPassage = TPhraseTag(CRelIndex(), 1));
	void startAbsoluteMode(TPhraseTag tagPassage = TPhraseTag(CRelIndex(), 1));
	void reset();

signals:
	void modeChanged(bool bRelative);
	void gotoIndex(const TPhraseTag &tag);

private:
	void initialize();
	void CalcPassage();
	void setDirectReference(const CRelIndex &ndx);

private slots:
	void en_TestamentComboIndexChanged(int index);
	void en_BookChanged(int nBook);
	void en_ChapterChanged(int nChapter);
	void en_VerseChanged(int nVerse);
	void en_WordChanged(int nWord);
	void en_ReverseChanged(bool bReverse);
	void en_RefTypeChanged(int nType);
	void en_BookDirectChanged(int index);
	void en_ChapterDirectChanged(int index);
	void en_VerseDirectChanged(int index);
	void en_WordDirectChanged(int index);
	void en_PassageReferenceChanged(const TPhraseTag &tagPhrase);

// Data Private:
private:
	CBibleDatabasePtr m_pBibleDatabase;
	TPhraseTag m_tagStartRef;	// Starting index		-- Will be unset if in Absolute Mode
	TPhraseTag m_tagPassage;	// Index of passage
	unsigned int m_nTestament;
	unsigned int m_nBook;
	unsigned int m_nChapter;
	unsigned int m_nVerse;
	unsigned int m_nWord;
	NavigatorRefTypeOptionFlags m_flagsRefTypes;		// Allowed Ref Types
	NAVIGATOR_REF_TYPE_ENUM m_nRefType;					// Selected Ref Type

// UI Private:
private:
	bool m_bDoingUpdate;		// True if combo boxes, etc, are being updated and change notifications should be ignored

#define begin_update()							\
			CBusyCursor iAmBusy(nullptr);		\
			bool bUpdateSave = m_bDoingUpdate;	\
			m_bDoingUpdate = true;
#define end_update()							\
			m_bDoingUpdate = bUpdateSave;

	CScriptureEdit *m_pEditVersePreview;
	Ui::CPassageNavigator ui;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(CPassageNavigator::NavigatorRefTypeOptionFlags)

// ============================================================================

#endif // PASSAGE_NAVIGATOR_H
