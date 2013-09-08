/****************************************************************************
**
** Copyright (C) 2013 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef NOTEKEYWORDWIDGET_H
#define NOTEKEYWORDWIDGET_H

#include "SubControls.h"

#include <QWidget>
#include <QString>
#include <QMenu>
#include <QAction>
#include <QList>
#include <QStringList>
#include <QAbstractListModel>
#include <QListView>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>

// ============================================================================

class CNoteKeywordModelItemData
{
public:
	CNoteKeywordModelItemData()
		:	m_bChecked(false)
	{ }
	CNoteKeywordModelItemData(const CNoteKeywordModelItemData &other)
		:	m_strKeyword(other.m_strKeyword),
			m_bChecked(other.m_bChecked)
	{ }
	CNoteKeywordModelItemData(const QString &strKeyword, bool bChecked)
		:	m_strKeyword(strKeyword),
			m_bChecked(bChecked)
	{ }

	QString m_strKeyword;
	bool m_bChecked;
};

// ============================================================================

class CNoteKeywordModelListView : public QListView
{
	Q_OBJECT

public:
	CNoteKeywordModelListView(QWidget *pParent = 0)
		:	QListView(pParent)
	{
		connect(this, SIGNAL(activated(const QModelIndex &)), this, SLOT(en_activated(const QModelIndex &)));
	}

	virtual ~CNoteKeywordModelListView() { }

signals:
	void currentKeywordChanged(const QString &strKeyword);

protected slots:
	void en_activated(const QModelIndex &index)
	{
		if (index.isValid()) {
			emit currentKeywordChanged(index.data(Qt::EditRole).toString());
		}
	}
};

// ============================================================================

typedef QList<CNoteKeywordModelItemData> CNoteKeywordModelItemDataList;

class CNoteKeywordModel : public QAbstractListModel
{
	Q_OBJECT

public:
	CNoteKeywordModel(QObject *pParent = 0);
	virtual ~CNoteKeywordModel();

	virtual int rowCount(const QModelIndex &zParent = QModelIndex()) const;

	virtual QVariant data(const QModelIndex &index, int role) const;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

	virtual QModelIndex findKeyword(const QString &strKeyword) const;

	virtual Qt::ItemFlags flags(const QModelIndex &index) const;

	virtual bool insertRows(int row, int count, const QModelIndex &zparent = QModelIndex());
	virtual bool removeRows(int row, int count, const QModelIndex &zParent = QModelIndex());

	virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

	const CNoteKeywordModelItemDataList &itemList() const;
	void setItemList(const CNoteKeywordModelItemDataList &aList);
	QStringList selectedKeywordList() const;
	void setKeywordList(const QStringList &lstSelectedKeywords = QStringList(), const QStringList &lstCompositeKeywords = QStringList());

//	virtual Qt::DropActions supportedDropActions() const;

	QMenu *contextMenu() { return &m_keywordContextMenu; }

signals:
	void changedNoteKeywords();

private slots:
	void en_selectAllKeywords();
	void en_clearKeywordSelection();
	void updateContextMenu();

// Data Private:
private:
	Q_DISABLE_COPY(CNoteKeywordModel)

	CNoteKeywordModelItemDataList m_lstKeywordData;

// UI Private:
private:
	QMenu m_keywordContextMenu;
	QAction *m_pActionSelectAllKeywords;
	QAction *m_pActionClearKeywordSelection;
};

// ============================================================================

enum KEYWORD_WIDGET_MODE_ENUM {
	KWME_EDITOR = 0,						// Widget works as a Keyword Editor
	KWME_SELECTOR = 1						// Widget works as a Keyword Selector (i.e. Read-only model)
};

class CKeywordComboBox : public CComboBox
{
	Q_OBJECT

public:
	CKeywordComboBox(QWidget *pParent = NULL)
		:	CComboBox(pParent)
	{ }
	virtual ~CKeywordComboBox() { }

	KEYWORD_WIDGET_MODE_ENUM mode() const { return m_nMode; }
	void setMode(KEYWORD_WIDGET_MODE_ENUM nMode) { m_nMode = nMode; }

	virtual void wheelEvent(QWheelEvent *pEvent);
	virtual void mousePressEvent(QMouseEvent *pEvent);
	virtual void mouseReleaseEvent(QMouseEvent *pEvent);
	virtual void keyPressEvent(QKeyEvent *pEvent);
	virtual void keyReleaseEvent(QKeyEvent *pEvent);

private:
	KEYWORD_WIDGET_MODE_ENUM m_nMode;
};

// ============================================================================

#include "ui_NoteKeywordWidget.h"

class CNoteKeywordWidget : public QWidget
{
	Q_OBJECT
	
public:
	explicit CNoteKeywordWidget(QWidget *parent = 0);
	~CNoteKeywordWidget();

	bool isAllKeywordsSelected() const;
	QStringList selectedKeywordList() const;
	void setKeywordList(const QStringList &lstSelectedKeywords = QStringList(), const QStringList &lstCompositeKeywords = QStringList());

	KEYWORD_WIDGET_MODE_ENUM mode() const { return m_nMode; }
	void setMode(KEYWORD_WIDGET_MODE_ENUM nMode);

	bool haveUnenteredKeywords() const;				// Returns true if the user has entered some text in the keyword editor, but hasn't pressed "enter" to make them take effect
	void enterKeywords() { en_keywordEntered(); }	// Simulate user pressing "enter" to enter keyword text

signals:
	void keywordListChanged();

private slots:
	void en_keywordEntered();
	void en_keywordListChanged();
	void en_keywordCurrentIndexChanged(const QString &text);

	void en_customContextMenuRequested(const QPoint &pos);
	void en_customContextMenuRequestedView(const QPoint &pos);

private:
	void setKeywordListPreview();

// Data Private:
private:
	CNoteKeywordModel *m_pKeywordModel;

// UI Private:
private:
	Ui::CNoteKeywordWidget ui;
	bool m_bDoingUpdate;
	KEYWORD_WIDGET_MODE_ENUM m_nMode;
};

// ============================================================================

#endif // NOTEKEYWORDWIDGET_H
