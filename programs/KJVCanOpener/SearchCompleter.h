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

#ifndef SEARCH_COMPLETER_H
#define SEARCH_COMPLETER_H

#include "dbstruct.h"
#include "PersistentSettings.h"

#include <QString>
#include <QCompleter>

// ============================================================================

// Forward Declarations:
class QWidget;
class QTextEdit;
class CParsedPhrase;
class CSearchStringListModel;
class CSearchStrongsDictionaryListModel;
class CSoundExSearchCompleterFilter;

// ============================================================================

class CSearchCompleter : public QCompleter
{
	Q_OBJECT

public:
	CSearchCompleter(CParsedPhrase &parsedPhrase, QWidget *parentWidget);
	CSearchCompleter(CDictionaryDatabasePtr pDictionary, const QTextEdit &editor, QWidget *parentWidget);
	CSearchCompleter(QWidget *parentWidget);
	virtual ~CSearchCompleter();

	virtual CSearchStringListModel *searchStringListModel() { return m_pSearchStringListModel; }
	virtual CSoundExSearchCompleterFilter *soundExFilterModel() { return m_pSoundExFilterModel; }

	virtual SEARCH_COMPLETION_FILTER_MODE_ENUM completionFilterMode() const { return m_nCompletionFilterMode; }
	virtual void setCompletionFilterMode(SEARCH_COMPLETION_FILTER_MODE_ENUM nCompletionFilterMode);

	virtual QString filterMatchString() const { return m_strFilterMatchString; }

	virtual void selectFirstMatchString();

	virtual void UpdateCompleter(const QTextEdit &editor);

public slots:
	virtual void setFilterMatchString();
	virtual void setWordsFromPhrase(bool bForceUpdate = false);

protected:
	SEARCH_COMPLETION_FILTER_MODE_ENUM m_nCompletionFilterMode;
	QString m_strFilterMatchString;

private:
	CSearchStringListModel *m_pSearchStringListModel;
	CSoundExSearchCompleterFilter *m_pSoundExFilterModel;
};

// ============================================================================

#if QT_VERSION < 0x050000

// Forward declares:
class CParsedPhrase;

// CComposingCompleter -- Needed to fix a bug in Qt 4.8.x QCompleter whereby
//		inputMethod events get redirected to the popup, but don't come back
//		to the editor because inputContext()->setFocusWidget() never gets
//		called again for the editor:
class CComposingCompleter : public CSearchCompleter
{
	Q_OBJECT

public:
	CComposingCompleter(CParsedPhrase &parsedPhrase, QWidget *parentWidget)
		:	CSearchCompleter(parsedPhrase, parentWidget)
	{

	}

	CComposingCompleter(CDictionaryDatabasePtr pDictionary, const QTextEdit &editor, QWidget *parentWidget)
		:	CSearchCompleter(pDictionary, editor, parentWidget)
	{

	}

	CComposingCompleter(QWidget *parentWidget)
		:	CSearchCompleter(parentWidget)
	{

	}

	~CComposingCompleter()
	{

	}

	virtual bool eventFilter(QObject *obj, QEvent *ev) override;
};

typedef CComposingCompleter SearchCompleter_t;

#else

typedef CSearchCompleter SearchCompleter_t;

#endif

// ============================================================================

class CStrongsDictionarySearchCompleter : public SearchCompleter_t
{
	Q_OBJECT

public:
	CStrongsDictionarySearchCompleter(CDictionaryDatabasePtr pDictionary, const QTextEdit &editor, QWidget *parentWidget);
	virtual ~CStrongsDictionarySearchCompleter() { }

	using CSearchCompleter::completionFilterMode;
	virtual void setCompletionFilterMode(SEARCH_COMPLETION_FILTER_MODE_ENUM nCompletionFilterMode) override { m_nCompletionFilterMode = nCompletionFilterMode; }

	using CSearchCompleter::filterMatchString;

	virtual void selectFirstMatchString() override;

public slots:
	virtual void setFilterMatchString() override;
	virtual void setWordsFromPhrase(bool bForceUpdate = false) override;

private:
	CSearchStrongsDictionaryListModel *m_pStrongsListModel;
};

// ============================================================================

#endif	// SEARCH_COMPLETER_H
