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

#include "ScriptureWebEngine.h"

#include "dbstruct.h"
#include "PhraseNavigator.h"
#include "PersistentSettings.h"
#include "Highlighter.h"
#include "myApplication.h"
#include "KJVCanOpener.h"

#include <QWebEngineSettings>
#include <QWebEngineUrlRequestInterceptor>
#include <QWebEngineUrlRequestInfo>
#include <QWebEngineUrlRequestJob>
#if QT_VERSION >= 0x060000
#include <QWebEngineProfile>
#endif

#include <QBuffer>
#include <QUrl>
#include <QString>
#include <QStringList>
#include <QTextDocument>

// ============================================================================

CKJPBSWebViewSchemeHandler::CKJPBSWebViewSchemeHandler(QObject *parent)
	:	QWebEngineUrlSchemeHandler(parent)
{
}

CKJPBSWebViewSchemeHandler::~CKJPBSWebViewSchemeHandler()
{
}

void CKJPBSWebViewSchemeHandler::requestStarted(QWebEngineUrlRequestJob *request)
{
	Q_ASSERT(request != nullptr);

	// We will be using URLs in the forms of:
	//	kjpbs://bible-uuid/RelIndex				<< here, the "hostname" will be the bible-uuid, and path will be /RelIndex
	//	kjpbs:///bible-uuid/RelIndex			<< here, the "hostname" will be empty, and path will be /bible-uuid/RelIndex
	//
	//	Where RelIndex is a Chapter RelIndex asAnchor string

	QUrl url = request->requestUrl();
	QString strPath = url.path();
	QStringList lstPath = strPath.split('/');
	if ((lstPath.size() == 2) && lstPath.at(0).isEmpty()) {		// Empty (0) means absolute path (currently the only allowed form)
		lstPath[0] = url.host();				// Switch (0) to bible-uuid, with (1) RelIndex
	} else if ((lstPath.size() == 3) && lstPath.at(0).isEmpty()) {	// Empty (0) means absolute path (currently the only allowed form)
		lstPath.removeAt(0);					// Make (0) bible-uuid, with (1) RelIndex
	} else {
		request->fail(QWebEngineUrlRequestJob::Error::UrlInvalid);
		return;
	}

	CBibleDatabasePtr pBibleDatabase = TBibleDatabaseList::instance()->atUUID(lstPath.at(0));
	if (pBibleDatabase.isNull()) {
		request->fail(QWebEngineUrlRequestJob::Error::UrlNotFound);
		return;
	}

	QTextDocument doc;
	CPhraseNavigator navigator(pBibleDatabase, doc);

	// Don't use defaultDocumentToChapterFlags here so we can
	//	suppress UserNotes and CrossRefs:
	QString strHTML = navigator.setDocumentToChapter(CRelIndex(lstPath.at(1)),
														CPhraseNavigator::TRO_Subtitles |
														CPhraseNavigator::TRO_Colophons |
														CPhraseNavigator::TRO_Superscriptions |
														CPhraseNavigator::TRO_Category |
														CPhraseNavigator::TRO_ScriptureBrowser |
														CPhraseNavigator::TRO_UseLemmas);
	int nPos = strHTML.indexOf("<style type=\"text/css\">\n");
	Q_ASSERT(nPos > -1);		// If these assert, update this search to match CPhraseNavigator::setDocumentToChapter()
	nPos = strHTML.indexOf("body", nPos);
	Q_ASSERT(nPos > -1);
	nPos = strHTML.indexOf("{", nPos);
	Q_ASSERT(nPos > -1);
	if (nPos > -1) {
		strHTML.insert(nPos+1, QString(" background-color:%1; color: %2;\n")
						.arg(CPersistentSettings::instance()->textBackgroundColor().name())
						.arg(CPersistentSettings::instance()->textForegroundColor().name()));
	}

	QBuffer *pResults = new QBuffer(request);		// Parent to job so it can free the result data
	pResults->setData(strHTML.toUtf8());
	request->reply("text/html", pResults);
}

// ============================================================================

CScriptureWebEngineView::CScriptureWebEngineView(CVerseListModel *pSearchResultsListModel, QWidget *pParent)
	:	QWebEngineView(pParent),
		m_pSearchResultsListModel(pSearchResultsListModel)
{
//	setWindowFlags(Qt::Tool | Qt::CustomizeWindowHint | Qt::BypassGraphicsProxyWidget | Qt::WindowTitleHint);
//	setWindowTitle(tr("King James Pure Bible Search", "MainMenu"));

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	connect(this, SIGNAL(loadFinished(bool)), this, SLOT(en_loadFinished(bool)));

	en_setFont(CPersistentSettings::instance()->fontScriptureBrowser());
	en_setTextBrightness(CPersistentSettings::instance()->invertTextBrightness(), CPersistentSettings::instance()->textBrightness());

	connect(CPersistentSettings::instance(), SIGNAL(fontChangedScriptureBrowser(const QFont &)), this, SLOT(en_setFont(const QFont &)));
	connect(CPersistentSettings::instance(), SIGNAL(changedTextBrightness(bool, int)), this, SLOT(en_setTextBrightness(bool, int)));
}

CScriptureWebEngineView::~CScriptureWebEngineView()
{
}

void CScriptureWebEngineView::en_loadFinished(bool bOK)
{
	if (bOK) {
		Q_ASSERT(page() != nullptr);
		if (page()) {
			// Chrome Hack to convert relative anchor hrefs to absolute,
			//	from https://stackoverflow.com/questions/49116986/in-page-href-in-qtwebview-fails-to-display-anything
			page()->runJavaScript(
							"var url = window.location.href;\n"
							"var links = document.querySelectorAll('a');\n"
							"for (index = 0; index < links.length; ++index) {\n"
							"	var link = links[index];\n"
							"	var href = link.getAttribute('href');\n"
							"	if (href && href[0] == '#') {\n"
							"		link.href = url + href;\n"
							"	}\n"
							"}\n"
						);
		}
	}
}

void CScriptureWebEngineView::en_setFont(const QFont& aFont)
{
#if QT_VERSION >= 0x060000
	// TODO : Fix this once Qt6 stabilizes.  Their documentation says to use
	//	QWebEngineSettings::defaultSettings(), yet that function doesn't exist
	//	and is commented out in the Qt Source Code.  Their commented out code,
	//	however, is what is here below since QWebEngineSettings::globalSettings()
	//	no longer exists either:
	QWebEngineSettings *pSettings = QWebEngineProfile::defaultProfile()->settings();
#else
	QWebEngineSettings *pSettings = QWebEngineSettings::globalSettings();
#endif
	Q_ASSERT(pSettings != nullptr);

	pSettings->setFontFamily(QWebEngineSettings::StandardFont, aFont.family());
	pSettings->setFontSize(QWebEngineSettings::MinimumFontSize, aFont.pointSize());
}

void CScriptureWebEngineView::en_setTextBrightness(bool bInvert, int nBrightness)
{
	Q_ASSERT(page() != nullptr);
	if (page()) {
		page()->setBackgroundColor(CPersistentSettings::textBackgroundColor(bInvert, nBrightness));
		reload();		// Needed to update the css
	}
}

void CScriptureWebEngineView::load(const QUrl &url)
{
	QString strPath = url.path();
	QStringList lstPath = strPath.split('/');
	if ((lstPath.size() == 2) && lstPath.at(0).isEmpty()) {		// Empty (0) means absolute path (currently the only allowed form)
		lstPath[0] = url.host();				// Switch (0) to bible-uuid, with (1) RelIndex
	} else if ((lstPath.size() == 3) && lstPath.at(0).isEmpty()) {	// Empty (0) means absolute path (currently the only allowed form)
		lstPath.removeAt(0);					// Make (0) bible-uuid, with (1) RelIndex
	} else {
		return;
	}

	CBibleDatabasePtr pBibleDatabase;

	Q_ASSERT(!g_pMyApplication.isNull());
	CKJVCanOpener *pCanOpener = g_pMyApplication->findCanOpenerFromChild<CScriptureWebEngineView>(this);
	Q_ASSERT(pCanOpener != nullptr);
	if (pCanOpener != nullptr) {
		pBibleDatabase = pCanOpener->bibleDatabase();
	}
	CSearchResultHighlighter searchResultsHighlighter(m_pSearchResultsListModel, CPersistentSettings::instance()->showExcludedSearchResultsInBrowser());

	if (pBibleDatabase.isNull()) {
		pBibleDatabase = TBibleDatabaseList::instance()->atUUID(lstPath.at(0));
		if (pBibleDatabase.isNull()) {
			return;
		}
	}

	QTextDocument doc;
	CPhraseNavigator navigator(pBibleDatabase, doc);

	// Don't use defaultDocumentToChapterFlags here so we can
	//	suppress UserNotes and CrossRefs:
	QString strHTML = navigator.setDocumentToChapter(CRelIndex(lstPath.at(1)),
														CPhraseNavigator::TRO_Subtitles |
														CPhraseNavigator::TRO_Colophons |
														CPhraseNavigator::TRO_Superscriptions |
														CPhraseNavigator::TRO_Category |
														CPhraseNavigator::TRO_ScriptureBrowser |
														CPhraseNavigator::TRO_UseLemmas,
													 &searchResultsHighlighter);
	int nPos = strHTML.indexOf("<style type=\"text/css\">\n");
	Q_ASSERT(nPos > -1);		// If these assert, update this search to match CPhraseNavigator::setDocumentToChapter()
	nPos = strHTML.indexOf("body", nPos);
	Q_ASSERT(nPos > -1);
	nPos = strHTML.indexOf("{", nPos);
	Q_ASSERT(nPos > -1);
	if (nPos > -1) {
		strHTML.insert(nPos+1, QString(" background-color:%1; color: %2;\n")
						.arg(CPersistentSettings::instance()->textBackgroundColor().name())
						.arg(CPersistentSettings::instance()->textForegroundColor().name()));
	}

	setHtml(strHTML, url);
}

// ============================================================================
