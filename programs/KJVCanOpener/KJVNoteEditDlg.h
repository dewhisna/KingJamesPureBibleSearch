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

#ifndef KJV_NOTE_EDIT_DLG_H
#define KJV_NOTE_EDIT_DLG_H

#include "dbstruct.h"
#include "UserNotesDatabase.h"

#include <QString>
#include <QAction>
#include <QDialog>
#include <QwwColorButton>
#include <QwwRichTextEdit>
#include <QPushButton>
#include <QAbstractButton>
#include <QSettings>
#include <QList>
#include <QStringList>
#include <QAbstractListModel>
#include <QListView>

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

typedef QList<CNoteKeywordModelItemData> CNoteKeywordModelItemDataList;

class CNoteKeywordModel : public QAbstractListModel
{
	Q_OBJECT

public:
	CNoteKeywordModel(const QStringList &lstSelectedKeywords = QStringList(), const QStringList &lstCompositeKeywords = QStringList(), QObject *pParent = 0);
	virtual ~CNoteKeywordModel();

	int rowCount(const QModelIndex &zParent = QModelIndex()) const;

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

//	virtual Qt::DropActions supportedDropActions() const;

signals:
	void changedNoteKeywords();

// Data Private:
private:
	Q_DISABLE_COPY(CNoteKeywordModel)

	CNoteKeywordModelItemDataList m_lstKeywordData;
};

// ============================================================================

namespace Ui {
	class CKJVNoteEditDlg;
}

class CKJVNoteEditDlg : public QDialog
{
	Q_OBJECT
	
public:
	explicit CKJVNoteEditDlg(CBibleDatabasePtr pBibleDatabase, QWidget *parent = 0);
	virtual ~CKJVNoteEditDlg();

	virtual void writeSettings(QSettings &settings, const QString &prefix = "UserNoteEditor");
	virtual void readSettings(QSettings &settings, const QString &prefix = "UserNoteEditor");

	CRelIndex locationIndex() const { return m_ndxLocation; }
	void setLocationIndex(const CRelIndex &ndxLocation);

	static QAction *actionUserNoteEditor();

protected:
	friend class CKJVCanOpener;			// Main App is Friend to create/set initial action
	static void setActionUserNoteEditor(QAction *pAction);

public slots:
	virtual void accept();
	virtual void reject();

private slots:
	void en_textChanged();
	void en_BackgroundColorPicked(const QColor &color);
	void en_ButtonClicked(QAbstractButton *button);
	void en_keywordEntered();
	void en_keywordListChanged();
	void en_keywordCurrentIndexChanged(const QString &text);

private:
	void setBackgroundColorPreview();
	void setKeywordListPreview();

private:
	static QAction *m_pActionUserNoteEditor;
	// ----
	Ui::CKJVNoteEditDlg *ui;
	QwwColorButton *m_pBackgroundColorButton;
	QwwRichTextEdit *m_pRichTextEdit;
	QPushButton *m_pDeleteNoteButton;
	CNoteKeywordModel *m_pKeywordModel;
	// ----
	CBibleDatabasePtr m_pBibleDatabase;
	// ----
	bool m_bDoingUpdate;
	bool m_bIsDirty;
	CRelIndex m_ndxLocation;
	CUserNoteEntry m_UserNote;
};

// ============================================================================

#endif // KJV_NOTE_EDIT_DLG_H
