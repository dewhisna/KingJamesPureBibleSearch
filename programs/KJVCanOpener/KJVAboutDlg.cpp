/****************************************************************************
**
** Copyright (C) 2012 Donna Whisnant, a.k.a. Dewtronics.
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
#include "ui_KJVAboutDlg.h"

#include "version.h"

#include <QApplication>
#include <QGraphicsScene>
#include <QPixmap>
#include <QPushButton>
#include <QMessageBox>

CKJVAboutDlg::CKJVAboutDlg(QWidget *parent) :
	QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint),
	ui(new Ui::CKJVAboutDlg)
{
	ui->setupUi(this);

	QPushButton *pLicenseButton = ui->buttonBox->addButton("&License", QDialogButtonBox::ActionRole);
	connect(pLicenseButton, SIGNAL(clicked()), this, SLOT(on_licenseDisplay()));
	QPushButton *pCloseButton = ui->buttonBox->button(QDialogButtonBox::Close);
	if (pCloseButton) pCloseButton->setDefault(true);

	QGraphicsScene *scene = new QGraphicsScene(this);

	m_pKJVCan = scene->addPixmap(QPixmap(":/res/can-of-KJV.png"));
	m_pKJVCan->setToolTip(VER_COMMENTS_STR);
	m_pBethelChurch = scene->addPixmap(QPixmap(":/res/church02-e.jpg"));
	m_pAppTitle = scene->addText("King James Can Opener - Version: " + qApp->applicationVersion(), QFont("Times New Roman", 21));
	m_pAppTitle->setTextInteractionFlags(Qt::TextBrowserInteraction);
	m_pBroughtToYouBy = scene->addText("Brought to you by the fervent prayers of Bethel Church; Festus, MO", QFont("Script MT Bold", 12));
	m_pBroughtToYouBy->setTextInteractionFlags(Qt::TextBrowserInteraction);
	m_pBethelURL = scene->addText("");
	m_pBethelURL->setHtml(QString("<html><head><meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"><style type=\"text/css\"><!-- A { text-decoration:none } %s --></style></head><body style=\" font-family:'Times New Roman'; font-size:12pt; font-weight:400; font-style:normal;\"><a href=\"") + QString(VER_URL_STR) + QString("\">Click Here to Visit Bethel Church</a></body></html>"));
	m_pBethelURL->setOpenExternalLinks(true);
	m_pBethelURL->setTextInteractionFlags(Qt::TextBrowserInteraction);
	m_pBethelChurch->setPos(m_pKJVCan->pos().x() + m_pKJVCan->boundingRect().width(), m_pKJVCan->pos().y());
	m_pAppTitle->setPos(m_pBethelChurch->pos().x(), m_pBethelChurch->pos().y() + m_pBethelChurch->boundingRect().height());
	m_pBroughtToYouBy->setPos(m_pAppTitle->pos().x(), m_pAppTitle->pos().y() + m_pAppTitle->boundingRect().height());
	m_pBethelURL->setPos(m_pBroughtToYouBy->pos().x(), m_pBroughtToYouBy->pos().y() + m_pBroughtToYouBy->boundingRect().height());
	ui->graphicsView->setScene(scene);
	adjustSize();
}

CKJVAboutDlg::~CKJVAboutDlg()
{
	delete ui;
}

void CKJVAboutDlg::on_licenseDisplay()
{
	QMessageBox::information(this, "About King James Can Opener License",
						"This program is free software; you can redistribute it and/or modify it under the terms "
						"of the GNU General Public License as published by the Free Software Foundation; either "
						"version 3 of the License, or (at your option) any later version.\n\n"
						"This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; "
						"without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  "
						"See the GNU General Public License for more details.\n\n"
						"You should have received a copy of the GNU General Public License along with this program; "
						"if not, write to the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.\n\n"
						"Copyright (C) 2012 Donna Whisnant, a.k.a. Dewtronics.\n"
						"Contact: http://www.dewtronics.com/\n"
						"Written and Developed for Bethel Church, Festus, MO.");
}
