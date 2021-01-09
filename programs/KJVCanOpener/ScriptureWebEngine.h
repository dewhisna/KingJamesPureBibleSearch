/****************************************************************************
**
** Copyright (C) 2020 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef SCRIPTUREWEBENGINE_H
#define SCRIPTUREWEBENGINE_H

// ============================================================================

#include <QWebEngineView>
#include <QWebEngineUrlScheme>
#include <QWebEngineUrlSchemeHandler>
#include <QWebEngineProfile>

class QWebEngineUrlRequestJob;

// ============================================================================

//
// Custom CKJPBSWebViewSchemeHandler
//

class CKJPBSWebViewSchemeHandler : public QWebEngineUrlSchemeHandler
{
public:
	class CKJPBSWebViewUrlScheme : public QWebEngineUrlScheme
	{
	protected:
		CKJPBSWebViewUrlScheme()
			:	QWebEngineUrlScheme("kjpbs")
		{
			setSyntax(QWebEngineUrlScheme::Syntax::Path);
			setFlags(QWebEngineUrlScheme::Flag::LocalScheme);
		}

	public:
		static void registerScheme()
		{
			QWebEngineUrlScheme::registerScheme(scheme());
		}

		static CKJPBSWebViewUrlScheme &scheme()
		{
			static CKJPBSWebViewUrlScheme theScheme;
			return theScheme;
		}
	};

	// ------------------------------------------------------------------------

protected:
	CKJPBSWebViewSchemeHandler(QObject *parent = nullptr);
public:
	virtual ~CKJPBSWebViewSchemeHandler();
	virtual void requestStarted(QWebEngineUrlRequestJob *request) override;

	static void installUrlSchemeHandler()
	{
		QWebEngineProfile::defaultProfile()->installUrlSchemeHandler(CKJPBSWebViewUrlScheme::scheme().name(), &handler());
	}

	static CKJPBSWebViewSchemeHandler &handler()
	{
		static CKJPBSWebViewSchemeHandler schemeHandler;
		return schemeHandler;
	}
};


// ============================================================================

//
// CScriptureWebEngineView - Base Class for viewing interlinear text with Lemmas and Morphology
//

class CVerseListModel;			// Forward declaration

class CScriptureWebEngineView : public QWebEngineView
{
	Q_OBJECT

public:
	explicit CScriptureWebEngineView(CVerseListModel *pSearchResultsListModel, QWidget *pParent = nullptr);
	virtual ~CScriptureWebEngineView();

	void load(const QUrl &url);

protected slots:
	void en_loadFinished(bool bOK);

	void en_setFont(const QFont& aFont);
	void en_setTextBrightness(bool bInvert, int nBrightness);

private:
	CVerseListModel *m_pSearchResultsListModel;
};

// ============================================================================

#endif	// SCRIPTUREWEBENGINE_H

