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

	enum OPERATOR_MODE_ENUM {
		OME_OR = 0,
		OME_AND = 1
	};

signals:
	void changedSearchScope(CKJVSearchCriteria::SEARCH_SCOPE_MODE_ENUM mode);
	void changedOperator(CKJVSearchCriteria::OPERATOR_MODE_ENUM mode);
	void addSearchPhraseClicked();

private slots:
	void on_changeSearchScope(int ndx);
	void on_changeOperator();

// Data Private:
private:
	SEARCH_SCOPE_MODE_ENUM m_nSearchScope;
	OPERATOR_MODE_ENUM m_nOperator;

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
