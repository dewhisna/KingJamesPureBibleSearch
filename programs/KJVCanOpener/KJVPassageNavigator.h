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
	~CKJVPassageNavigator();

	CRelIndex passage() const { return m_ndxPassage; }
	void setPassage(const CRelIndex &ndx);

private:
	void CalcPassage();

private slots:
	void TestamentComboIndexChanged(int index);
	void BookChanged(const QString &strBook);
	void ChapterChanged(const QString &strChapter);
	void VerseChanged(const QString &strVerse);
	void WordChanged(const QString &strWord);

// Data Private:
private:
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
