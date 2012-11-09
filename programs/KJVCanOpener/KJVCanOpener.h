#ifndef KJVCANOPENER_H
#define KJVCANOPENER_H

#include <QMainWindow>
#include <QListWidget>
#include <QListWidgetItem>
#include <QModelIndex>
#include <QScrollArea>
#include <QAction>

#include <assert.h>

#include "dbstruct.h"
#include "KJVSearchPhraseEdit.h"

class CSearchPhraseScrollArea : public QScrollArea
{
public:
	CSearchPhraseScrollArea( QWidget *parent=NULL)
		: QScrollArea(parent)
	{ }
	virtual ~CSearchPhraseScrollArea() { }

	virtual QSize minimumSizeHint() const;
	virtual QSize sizeHint() const;
};


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

	void Initialize(CRelIndex nInitialIndex = CRelIndex(1,1,0,0));		// Default initial location is Genesis 1

protected slots:
	void on_phraseChanged(const CParsedPhrase &phrase);
	void on_SearchResultDoubleClick(const QModelIndex &index);

	void on_PassageNavigatorTriggered();

// UI Private:
private:
	QAction *m_pActionJump;			// Jump to passage via Passage Navigator
	Ui::CKJVCanOpener *ui;
};

#endif // KJVCANOPENER_H
