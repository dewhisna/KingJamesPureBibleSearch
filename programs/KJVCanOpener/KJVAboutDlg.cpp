/****************************************************************************
**
** Copyright (C) 2012-2020 Donna Whisnant, a.k.a. Dewtronics.
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

#include "KJVAboutDlg.h"

#include "ReportError.h"
#include "version.h"

#include <QApplication>
#include <QGraphicsScene>
#include <QPixmap>
#include <QPushButton>
#include <QMessageBox>
#include <QScrollBar>
#include <QTimer>

// ============================================================================

#if QT_VERSION < 0x050000
#include <QTextDocument>			// Needed for Qt::escape, which is in this header, not <Qt> as is assistant says

static inline QString htmlEscape(const QString &aString)
{
	return Qt::escape(aString);
}
#else
static inline QString htmlEscape(const QString &aString)
{
	return aString.toHtmlEscaped();
}
#endif

// ============================================================================

CKJVAboutDlg::CKJVAboutDlg(QWidget *parent) :
	QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint),
	m_pBethelChurch(nullptr),
	m_pAppTitle(nullptr),
	m_pExtraVersionInfo(nullptr),
	m_pAppSpecialVersion(nullptr),
	m_pAppBuildDateTime(nullptr),
	m_pQtVersion(nullptr),
	m_pBroughtToYouBy(nullptr),
	m_pBethelURL(nullptr)
{
	ui.setupUi(this);

#ifdef USE_ASYNC_DIALOGS
	setAttribute(Qt::WA_DeleteOnClose);
#endif

	QPushButton *pLicenseButton = ui.buttonBox->addButton(tr("&License", "AboutBox"), QDialogButtonBox::ActionRole);
	connect(pLicenseButton, SIGNAL(clicked()), this, SLOT(en_licenseDisplay()));
	QPushButton *pCloseButton = ui.buttonBox->button(QDialogButtonBox::Close);
	if (pCloseButton) pCloseButton->setDefault(true);

	QGraphicsScene *scene = new QGraphicsScene(this);

	m_pBethelChurch = scene->addPixmap(QPixmap(":/res/church02-e.jpg") /* .scaledToWidth(665) */ );
	m_pAppTitle = scene->addText(tr("King James Pure Bible Search - Version: ", "AboutBox") + qApp->applicationVersion(), QFont("Times New Roman", 21));
	m_pAppTitle->setTextInteractionFlags(Qt::TextBrowserInteraction);
#if defined(EMSCRIPTEN)
	m_pExtraVersionInfo = scene->addText(tr("Lite Version", "AboutBox"), QFont("Times New Roman", 10));
	m_pExtraVersionInfo->setTextInteractionFlags(Qt::TextBrowserInteraction);
#elif defined(VNCSERVER)
	m_pExtraVersionInfo = scene->addText(tr("Lite Version", "AboutBox"), QFont("Times New Roman", 10));
	m_pExtraVersionInfo->setTextInteractionFlags(Qt::TextBrowserInteraction);
#else
	m_pExtraVersionInfo = nullptr;
#endif
	QString strSpecialVersion(SPECIAL_BUILD ? QString(VER_SPECIALVERSION_STR) : QString());
	if (!strSpecialVersion.isEmpty()) {
		m_pAppSpecialVersion = scene->addText(QString("%1").arg(strSpecialVersion), QFont("Times New Roman", 10));
		m_pAppSpecialVersion->setTextInteractionFlags(Qt::TextBrowserInteraction);
	}
	QString strBuildDate(VER_BUILD_DATE_STR);
	QString strBuildTime(VER_BUILD_TIME_STR);
	if (!strBuildDate.isEmpty()) {
		m_pAppBuildDateTime = scene->addText(QString("%1: %2  %3").arg(tr("Build", "AboutBox")).arg(strBuildDate).arg(strBuildTime), QFont("Times New Roman", 10));
		m_pAppBuildDateTime->setTextInteractionFlags(Qt::TextBrowserInteraction);
	}
	m_pQtVersion = scene->addText(tr("Based on Qt Version %1", "AboutBox").arg(QT_VERSION_STR), QFont("Times New Roman", 10));
	m_pBroughtToYouBy = scene->addText(tr("Brought to you by the fervent prayers of Bethel Church; Festus, MO", "AboutBox"), QFont("Script MT Bold", 12));
	m_pBroughtToYouBy->setTextInteractionFlags(Qt::TextBrowserInteraction);
	m_pBethelURL = scene->addText("");
	m_pBethelURL->setHtml(QString("<html><head><meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"><style type=\"text/css\"><!-- A { text-decoration:none } %s --></style></head><body style=\" font-family:'Times New Roman'; font-size:12pt; font-weight:400; font-style:normal;\"><a href=\"") + QString(VER_URL_STR) + QString("\">") + htmlEscape(tr("Click Here to Visit Bethel Church", "AboutBox")) + QString("</a></body></html>"));
	m_pBethelURL->setOpenExternalLinks(true);
	m_pBethelURL->setTextInteractionFlags(Qt::TextBrowserInteraction);
	// --------
	qreal nXCenterLine = m_pBethelChurch->pos().x() + (m_pBethelChurch->boundingRect().width() / 2);
	qreal nXoneFourth = m_pBethelChurch->pos().x() + (m_pBethelChurch->boundingRect().width() / 4);
	qreal nXthreeFourth = m_pBethelChurch->pos().x() + ((m_pBethelChurch->boundingRect().width() * 3) / 4);
	qreal nYPos = m_pBethelChurch->pos().y() + m_pBethelChurch->boundingRect().height();
	m_pBethelURL->setPos(nXCenterLine - (m_pBethelURL->boundingRect().width() / 2), nYPos);
	nYPos += m_pBethelURL->boundingRect().height();
	m_pAppTitle->setPos(nXCenterLine - (m_pAppTitle->boundingRect().width() / 2), nYPos);
	nYPos += m_pAppTitle->boundingRect().height();
	if (m_pExtraVersionInfo) {
		m_pExtraVersionInfo->setPos(nXCenterLine - (m_pExtraVersionInfo->boundingRect().width() / 2), nYPos);
		nYPos += m_pExtraVersionInfo->boundingRect().height();
	}
	if (m_pAppSpecialVersion) {
		m_pAppSpecialVersion->setPos(nXCenterLine - (m_pAppSpecialVersion->boundingRect().width() / 2), nYPos);
		nYPos += m_pAppSpecialVersion->boundingRect().height();
	}
	qreal nMaxVersionHeight = 0.0;
	if (m_pAppBuildDateTime) {
		m_pAppBuildDateTime->setPos(nXoneFourth - (m_pAppBuildDateTime->boundingRect().width() / 2), nYPos);
		nMaxVersionHeight = qMax(nMaxVersionHeight, m_pAppBuildDateTime->boundingRect().height());
	}
	if (m_pQtVersion) {
		m_pQtVersion->setPos(nXthreeFourth - (m_pQtVersion->boundingRect().width() / 2), nYPos);
		nMaxVersionHeight = qMax(nMaxVersionHeight, m_pQtVersion->boundingRect().height());
	}
	nYPos += nMaxVersionHeight;
	m_pBroughtToYouBy->setPos(nXCenterLine - (m_pBroughtToYouBy->boundingRect().width() / 2), nYPos);
	nYPos += m_pBroughtToYouBy->boundingRect().height();
	ui.graphicsView->setScene(scene);
	updateGeometry();
	adjustSize();

	if (ui.graphicsView->verticalScrollBar())
		ui.graphicsView->verticalScrollBar()->setValue(ui.graphicsView->verticalScrollBar()->maximum());

	// --------------------------------------------------------------

#ifndef Q_OS_MAC
	setWindowModality(Qt::WindowModal);		// Only block our parentCanOpener, not the whole app
#endif

	// Note:  The minimumSizeHint isn't computed until the
	//	event loop runs, so just calling adjustSize here has
	//	no effect.  So, we'll setup a dummy timer and
	//	trigger it later in the event stack:
	QTimer::singleShot(0, this, SLOT(en_resizeMe()));
}

CKJVAboutDlg::~CKJVAboutDlg()
{

}

void CKJVAboutDlg::en_resizeMe()
{
	adjustSize();

	QWidget *pParentWidget = parentWidget();
	if (pParentWidget != nullptr) {
		QPoint ptParent = pParentWidget->mapToGlobal(pParentWidget->rect().center());
		move(ptParent.x() - width()/2, ptParent.y() - height()/2);
	}
}

void CKJVAboutDlg::en_licenseDisplay()
{
	const QString strLicenseInfo =
			tr("This program is free software; you can redistribute it and/or modify it under the terms "
			"of the GNU General Public License as published by the Free Software Foundation; either "
			"version 3 of the License, or (at your option) any later version.\n\n"
			"This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; "
			"without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  "
			"See the GNU General Public License for more details.\n\n"
			"You should have received a copy of the GNU General Public License along with this program; "
			"if not, write to the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.\n\n"
			"Copyright (C) 2012-2020 Donna Whisnant, a.k.a. Dewtronics.\n"
			"Contact: http://www.dewtronics.com/\n"
			"Written and Developed for Bethel Church, Festus, MO.", "AboutBox");
	const QString strTitle = tr("About King James Pure Bible Search License", "AboutBox");

	displayInformation(this, strTitle, strLicenseInfo);
}

// ============================================================================
