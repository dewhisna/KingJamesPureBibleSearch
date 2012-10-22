#ifndef KJVCANOPENER_H
#define KJVCANOPENER_H

#include <QMainWindow>
#include <QListWidget>
#include <QListWidgetItem>
#include <QModelIndex>

#include <assert.h>

#include "dbstruct.h"
#include "KJVSearchPhraseEdit.h"

/*
class CPhraseEditListWidgetItem : public QListWidgetItem
{
public:
	CPhraseEditListWidgetItem(QListWidget *pParentView = NULL)
		:	QListWidgetItem(pParentView),
			m_widgetPhraseEdit(NULL)
	{
//		pParentView->addItem(this);
//		pParentView->setItemWidget(this, &m_widgetPhraseEdit);
		m_widgetPhraseEdit = new CKJVSearchPhraseEdit(pParentView);
	}

	~CPhraseEditListWidgetItem()
	{

	}

	CKJVSearchPhraseEdit *m_widgetPhraseEdit;
};
*/

// ============================================================================

namespace Ui {
class CKJVCanOpener;
}

class CKJVCanOpener : public QMainWindow
{
	Q_OBJECT

public:
	explicit CKJVCanOpener(QWidget *parent = 0);
	~CKJVCanOpener();

	void Initialize(uint32_t nInitialIndex = CRelIndex(1,1,1,1).index());	// Default initial location is the first word of Genesis 1:1)

protected slots:
	void on_phraseChanged(const CParsedPhrase &phrase);
	void on_SearchResultDoubleClick(const QModelIndex &index);

// UI Private:
private:
	Ui::CKJVCanOpener *ui;
};

#endif // KJVCANOPENER_H
