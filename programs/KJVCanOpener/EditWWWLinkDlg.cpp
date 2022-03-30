/****************************************************************************
**
** Copyright (C) 2014-2022 Donna Whisnant, a.k.a. Dewtronics.
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

#include "EditWWWLinkDlg.h"

#include <QPushButton>
#include <QDialogButtonBox>
#include <QUrl>
#include <QTimer>

// ============================================================================

CEditWWWLinkDlg::CEditWWWLinkDlg(const QString strURL, QWidget *pParent)
	:	QDialog(pParent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint),
		m_strURL(strURL)
{
	ui.setupUi(this);

	// --------------------------------------------------------------

	Q_ASSERT(ui.buttonBox->button(QDialogButtonBox::Ok) != nullptr);
	ui.buttonBox->button(QDialogButtonBox::Ok)->setIcon(QIcon(":/res/ok_blue-24.png"));
	Q_ASSERT(ui.buttonBox->button(QDialogButtonBox::Cancel) != nullptr);
	ui.buttonBox->button(QDialogButtonBox::Cancel)->setIcon(QIcon(":/res/cancel-24.png"));

	// Connect before setting the text so we'll trigger our slot:
	connect(ui.editWWWLink, SIGNAL(textChanged(const QString &)), this, SLOT(en_textChanged(const QString &)));
	ui.editWWWLink->setText(strURL);
	ui.editWWWLink->setFocus();
	QTimer::singleShot(0, this, SLOT(en_unselect()));			// Have to unselect after we've displayed and the editor has finished updating itself

	// --------------------------------------------------------------

#ifndef Q_OS_MAC
	setWindowModality(Qt::WindowModal);		// Only block our parentCanOpener, not the whole app
#endif
}

CEditWWWLinkDlg::~CEditWWWLinkDlg()
{

}

void CEditWWWLinkDlg::accept()
{
	if (m_bIsValid) QDialog::accept();
}

void CEditWWWLinkDlg::reject()
{
	QDialog::reject();
}

void CEditWWWLinkDlg::en_textChanged(const QString &strURL)
{
	QUrl aURL(strURL, QUrl::TolerantMode);
	QString strScheme = aURL.scheme();
	m_strURL = aURL.toString(QUrl::RemovePassword);		// Get the corrected URL and help user with security by removing passwords
	m_bIsValid = aURL.isValid();

	if ((strScheme.compare("http", Qt::CaseInsensitive) != 0) &&
		(strScheme.compare("https", Qt::CaseInsensitive) != 0) &&
		(strScheme.compare("ftp", Qt::CaseInsensitive) != 0) &&
		(strScheme.compare("ftps", Qt::CaseInsensitive) != 0) &&
		(strScheme.compare("sftp", Qt::CaseInsensitive) != 0)) m_bIsValid = false;

	Q_ASSERT(ui.buttonBox->button(QDialogButtonBox::Ok) != nullptr);
	ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(m_bIsValid);
}

void CEditWWWLinkDlg::en_unselect()
{
	ui.editWWWLink->setSelection(ui.editWWWLink->text().length(), 0);
}

// ============================================================================
