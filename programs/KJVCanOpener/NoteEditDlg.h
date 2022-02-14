/****************************************************************************
**
** Copyright (C) 2013-2020 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef NOTE_EDIT_DLG_H
#define NOTE_EDIT_DLG_H

#include "dbstruct.h"
#include "UserNotesDatabase.h"

#include <QDialog>
#include <QString>
#include <QwwColorButton>
#include <QwwRichTextEdit>
#include <QPushButton>
#include <QAbstractButton>
#include <QSettings>
#include <QPointer>

// ============================================================================

#include "ui_NoteEditDlg.h"

class CNoteEditDlg : public QDialog
{
	Q_OBJECT
	
public:
	explicit CNoteEditDlg(CBibleDatabasePtr pBibleDatabase, CUserNotesDatabasePtr pUserNotesDatabase, QWidget *parent = nullptr);
	virtual ~CNoteEditDlg();

	virtual void writeSettings(QSettings &settings, const QString &prefix = "UserNoteEditor");
	virtual void readSettings(QSettings &settings, const QString &prefix = "UserNoteEditor");

	CRelIndex locationIndex() const { return m_ndxLocation; }
	void setLocationIndex(const CRelIndex &ndxLocation);

public slots:
	virtual void accept() override;
	virtual void reject() override;

private slots:
	void en_textChanged();
	void en_BackgroundColorPicked(const QColor &color);
	void en_setDefaultNoteBackgroundColor();
	void en_ButtonClicked(QAbstractButton *button);
	void en_keywordListChanged();
	void en_clickedInsertReferenceLink();
	void en_clickedInsertWWWLink();

private:
	void setBackgroundColorPreview();
	CRelIndex navigateCrossRef(const CRelIndex &ndxStart);		// Bring up navigator at specified starting location for entering a ref-link and return selected ref.  If user cancels, returns CRelIndex()
	QString getWWWLink(const QString &strURL);

private:
	QwwColorButton *m_pBackgroundColorButton;
	QwwRichTextEdit *m_pRichTextEdit;
	QPushButton *m_pDeleteNoteButton;
	// ----
	CBibleDatabasePtr m_pBibleDatabase;
	CUserNotesDatabasePtr m_pUserNotesDatabase;
	// ----
	bool m_bDoingUpdate;
	bool m_bIsDirty;
	CRelIndex m_ndxLocation;
	CRelIndex m_ndxLastRefLink;
	CUserNoteEntry m_UserNote;
	bool m_bHaveGeometry;
	// ----
	Ui::CNoteEditDlg ui;
};

// ============================================================================

// SmartPointer classes needed, particularly for stack instantiated dialogs, since
//		this dialog is only WindowModal and the parent can get deleted during an
//		app close event, causing an attempted double-free which leads to a crash:
class CNoteEditDlgPtr : public QPointer<CNoteEditDlg>
{
public:
	CNoteEditDlgPtr(CBibleDatabasePtr pBibleDatabase, CUserNotesDatabasePtr pUserNotesDatabase, QWidget *parent = nullptr)
		:	QPointer<CNoteEditDlg>(new CNoteEditDlg(pBibleDatabase, pUserNotesDatabase, parent))
	{

	}

	virtual ~CNoteEditDlgPtr()
	{
		if (!isNull()) delete data();
	}
};

// ============================================================================

#endif // NOTE_EDIT_DLG_H
