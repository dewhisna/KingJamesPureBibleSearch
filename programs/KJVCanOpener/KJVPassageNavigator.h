#ifndef KJVPASSAGENAVIGATOR_H
#define KJVPASSAGENAVIGATOR_H

#include "dbstruct.h"

#include <QWidget>

namespace Ui {
class CKJVPassageNavigator;
}

class CKJVPassageNavigator : public QWidget
{
	Q_OBJECT

public:
	explicit CKJVPassageNavigator(QWidget *parent = 0);
	virtual ~CKJVPassageNavigator();

	TPhraseTag passage() const { return m_tagPassage; }
	void setPassage(const TPhraseTag &tag);
	TPhraseTag startRef() const { return m_tagStartRef; }

	bool isReversed() const;
	bool isRelative() const { return m_tagStartRef.first.isSet(); }
	bool isAbsolute() const { return (!m_tagStartRef.first.isSet()); }

public slots:
	void startRelativeMode(TPhraseTag tagStart = TPhraseTag(CRelIndex(), 1), TPhraseTag tagPassage = TPhraseTag(CRelIndex(), 1));
	void startRelativeMode(TPhraseTag tagStart, bool bReverse, TPhraseTag tagPassage = TPhraseTag(CRelIndex(), 1));
	void startAbsoluteMode(TPhraseTag tagPassage = TPhraseTag(CRelIndex(), 1));
	void reset();

signals:
	void modeChanged(bool bRelative);
	void gotoIndex(const TPhraseTag &tag);

private:
	void CalcPassage();

private slots:
	void TestamentComboIndexChanged(int index);
	void BookChanged(const QString &strBook);
	void ChapterChanged(const QString &strChapter);
	void VerseChanged(const QString &strVerse);
	void WordChanged(const QString &strWord);
	void on_ReverseChanged(bool bReverse);

// Data Private:
private:
	TPhraseTag m_tagStartRef;	// Starting index		-- Will be unset if in Absolute Mode
	TPhraseTag m_tagPassage;	// Index of passage
	unsigned int m_nTestament;
	unsigned int m_nBook;
	unsigned int m_nChapter;
	unsigned int m_nVerse;
	unsigned int m_nWord;

// UI Private:
private:
	bool m_bDoingUpdate;		// True if combo boxes, etc, are being updated and change notifications should be ignored

#define begin_update()							\
			bool bUpdateSave = m_bDoingUpdate;	\
			m_bDoingUpdate = true;
#define end_update()							\
			m_bDoingUpdate = bUpdateSave;

	Ui::CKJVPassageNavigator *ui;
};

#endif // KJVPASSAGENAVIGATOR_H
