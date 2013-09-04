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

#ifndef PASSAGEREFERENCEWIDGET_H
#define PASSAGEREFERENCEWIDGET_H

#include "dbstruct.h"

#include <QWidget>
#include <QList>
#include <QString>

namespace Ui {
	class CPassageReferenceWidget;
}

class CPassageReferenceWidget : public QWidget
{
	Q_OBJECT
	
public:
	explicit CPassageReferenceWidget(QWidget *parent = 0);
	~CPassageReferenceWidget();

	void initialize(CBibleDatabasePtr pBibleDatabase);

	void clear();
	TPhraseTag phraseTag() const { return m_tagPhrase; }

signals:
	void passageReferenceChanged(const TPhraseTag &tagPhrase);
	void enterPressed();

protected slots:
	virtual void focusInEvent(QFocusEvent *event);
	virtual void keyPressEvent(QKeyEvent *event);

private slots:
	void en_PassageReferenceChanged(const QString &strText);

private:
	void buildSoundExTables();
	uint32_t resolveBook(const QString &strPreBook, const QString &strBook) const;

// Data Private:
private:
	CBibleDatabasePtr m_pBibleDatabase;
	TPhraseTag m_tagPhrase;
	QList<QStringList> m_lstBookSoundEx;			// Index of [nBk-1], List of Book SoundEx Value lists.  Each sublist has the SoundEx for the book name as well as all abbreviations

// UI Private:
private:
	Ui::CPassageReferenceWidget *ui;
};

#endif // PASSAGEREFERENCEWIDGET_H
