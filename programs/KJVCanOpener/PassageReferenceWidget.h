/****************************************************************************
**
** Copyright (C) 2013-2022 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef PASSAGE_REFERENCE_WIDGET_H
#define PASSAGE_REFERENCE_WIDGET_H

#include "dbstruct.h"

#include <QList>
#include <QString>
#include <QObject>

#include <QWidget>
#include <QPointer>

// ============================================================================

// Forward declarations:
class QMenu;
class QAction;
class CPassageReferenceResolver;

#include "ui_PassageReferenceWidget.h"

class CPassageReferenceWidget : public QWidget
{
	Q_OBJECT

public:
	explicit CPassageReferenceWidget(QWidget *parent = nullptr);
	~CPassageReferenceWidget();

	void initialize(CBibleDatabasePtr pBibleDatabase);

	bool hasFocusPassageReferenceEditor() const;

	QMenu *getEditMenu() { return m_pEditMenu; }

	void clear();
	TPhraseTag phraseTag() const { return m_tagPhrase; }

protected:
	virtual bool eventFilter(QObject *pObject, QEvent *pEvent) override;

signals:
	void activatedPassageReference();
	void passageReferenceChanged(const TPhraseTag &tagPhrase);
	void enterPressed();

public slots:
	void setPassageReference(const QString &strPassageReference);

protected slots:
	virtual void focusInEvent(QFocusEvent *event) override;
	virtual void keyPressEvent(QKeyEvent *event) override;
	void en_passageReferenceContextMenuRequested(const QPoint &pos);

private slots:
	void en_setMenuEnables(const QString &strText);
	void en_PassageReferenceChanged(const QString &strText);

// Data Private:
private:
	TPhraseTag m_tagPhrase;
	QPointer<CPassageReferenceResolver> m_pRefResolver;

// UI Private:
private:
	QMenu *m_pEditMenu;								// Edit menu for main screen when this editor is active
	QAction *m_pActionUndo;
	QAction *m_pActionRedo;
	QAction *m_pActionCut;
	QAction *m_pActionCopy;
	QAction *m_pActionPaste;
	QAction *m_pActionDelete;
	QAction *m_pActionSelectAll;
	Ui::CPassageReferenceWidget ui;
};

// ============================================================================

#endif // PASSAGE_REFERENCE_WIDGET_H
