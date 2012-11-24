#ifndef KJVCANOPENER_H
#define KJVCANOPENER_H

#include <QMainWindow>
#include <QListWidget>
#include <QListWidgetItem>
#include <QModelIndex>
#include <QScrollArea>
#include <QAction>
#include <QCloseEvent>
#include <QString>

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
	explicit CKJVCanOpener(const QString &strUserDatabase = QString(), QWidget *parent = 0);
	~CKJVCanOpener();

	void Initialize(CRelIndex nInitialIndex = CRelIndex(1,1,0,0));		// Default initial location is Genesis 1

protected:
	virtual void closeEvent(QCloseEvent * event);
	bool haveUserDatabase() const { return !m_strUserDatabase.isEmpty(); }

protected slots:
	void on_viewVerseHeading();
	void on_viewVerseRichText();

	void on_browserHistoryChanged();
	void on_clearBrowserHistory();
	void on_phraseChanged(const CParsedPhrase &phrase);
	void on_SearchResultActivated(const QModelIndex &index);		// Enter or double-click activated

	void on_PassageNavigatorTriggered();

	void on_HelpManual();
	void on_HelpAbout();

// Data Private:
private:
	QString m_strUserDatabase;

// UI Private:
private:
	bool m_bDoingUpdate;
	QAction *m_pActionShowVerseHeading;		// Toggle action to show verse heading only
	QAction *m_pActionShowVerseRichText;	// Toggle action to show verse richtext
	QAction *m_pActionNavBackward;	// Browser Navigate Backward
	QAction *m_pActionNavForward;	// Browser Navigate Forward
	QAction *m_pActionNavHome;		// Browser Navigate to History Home
	QAction *m_pActionNavClear;		// Clear Navigation History
	QAction *m_pActionJump;			// Jump to passage via Passage Navigator
	QAction *m_pActionAbout;		// About Application
	Ui::CKJVCanOpener *ui;
};

#endif // KJVCANOPENER_H
