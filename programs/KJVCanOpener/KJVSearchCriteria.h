#ifndef KJVSEARCHCRITERIA_H
#define KJVSEARCHCRITERIA_H

#include <QWidget>

namespace Ui {
class CKJVSearchCriteria;
}

class CKJVSearchCriteria : public QWidget
{
	Q_OBJECT

public:
	explicit CKJVSearchCriteria(QWidget *parent = 0);
	~CKJVSearchCriteria();

	enum SEARCH_SCOPE_MODE_ENUM {
		SSME_WHOLE_BIBLE = 0,
		SSME_TESTAMENT = 1,
		SSME_BOOK = 2,
		SSME_CHAPTER = 3,
		SSME_VERSE = 4
	};

	SEARCH_SCOPE_MODE_ENUM searchScopeMode() const { return m_nSearchScopeMode; }

signals:
	void changedSearchScopeMode(CKJVSearchCriteria::SEARCH_SCOPE_MODE_ENUM mode);
	void addSearchPhraseClicked();
	void copySearchPhraseSummary();

public slots:
	void enableCopySearchPhraseSummary(bool bEnable);

private slots:
	void on_changeSearchScopeMode(int ndx);

// Data Private:
private:
	SEARCH_SCOPE_MODE_ENUM m_nSearchScopeMode;

// UI Private:
private:
	bool m_bDoingUpdate;		// True if combo boxes, etc, are being updated and change notifications should be ignored

#define begin_update()							\
			bool bUpdateSave = m_bDoingUpdate;	\
			m_bDoingUpdate = true;
#define end_update()							\
			m_bDoingUpdate = bUpdateSave;

	Ui::CKJVSearchCriteria *ui;
};

#endif // KJVSEARCHCRITERIA_H
