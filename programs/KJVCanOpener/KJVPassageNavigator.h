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

	CRelIndex passage() const { return m_ndxPassage; }
	void setPassage(const CRelIndex &ndx);
	CRelIndex startRef() const { return m_ndxStartRef; }

	bool isReversed() const;
	bool isRelative() const { return m_ndxStartRef.isSet(); }
	bool isAbsolute() const { return (!m_ndxStartRef.isSet()); }
	void startRelativeMode(CRelIndex ndxStart = CRelIndex(), CRelIndex ndxPassage = CRelIndex());
	void startRelativeMode(CRelIndex ndxStart, bool bReverse, CRelIndex ndxPassage = CRelIndex());
	void startAbsoluteMode(CRelIndex ndxPassage = CRelIndex());

signals:
	void modeChanged(bool bRelative);

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
	CRelIndex m_ndxStartRef;	// Starting index		-- Will be unset if in Absolute Mode
	CRelIndex m_ndxPassage;		// Index of passage
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
