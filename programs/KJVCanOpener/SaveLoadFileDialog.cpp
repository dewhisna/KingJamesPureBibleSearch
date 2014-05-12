/****************************************************************************
**
** Copyright (C) 2014 Donna Whisnant, a.k.a. Dewtronics.
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

#include "SaveLoadFileDialog.h"

#include <QFileInfo>
#include <QPointer>
#if QT_VERSION < 0x050000
#include <QDesktopServices>
#else
#include <QStandardPaths>
#endif

// ============================================================================

QString CSaveLoadFileDialog::m_strLastPath;

// ============================================================================

QString CSaveLoadFileDialog::getSaveFileName(QWidget *parent,
											const QString &caption,
											const QString &dir,
											const QString &filter,
											const QString &strDefaultSuffix,
											QString *selectedFilter,
											QFileDialog::Options options)
{
	if (lastPath().isEmpty()) setLastPath(dir);

	QPointer<QFileDialog> pDlg = new QFileDialog(parent, caption, lastPath(), filter);
	pDlg->setFileMode(QFileDialog::AnyFile);
	pDlg->setAcceptMode(QFileDialog::AcceptSave);
	pDlg->setDefaultSuffix(strDefaultSuffix);
	pDlg->setOptions(options);
	if (selectedFilter) pDlg->selectNameFilter(*selectedFilter);
	if (pDlg->exec() == QDialog::Accepted) {
		if (pDlg.data() != NULL) {
			QString strFilename = pDlg->selectedFiles().value(0);
			if (!strFilename.isEmpty()) setLastPath(strFilename);
			return strFilename;
		}
	}
	if (pDlg) delete pDlg;

	return QString();
}

QString CSaveLoadFileDialog::getOpenFileName(QWidget *parent,
											const QString &caption,
											const QString &dir,
											const QString &filter,
											QString *selectedFilter,
											QFileDialog::Options options)
{
	if (lastPath().isEmpty()) setLastPath(dir);

	QString strFilename = QFileDialog::getOpenFileName(parent, caption, lastPath(), filter, selectedFilter, options);
	if (!strFilename.isEmpty()) setLastPath(strFilename);
	return strFilename;
}

void CSaveLoadFileDialog::setLastPath(const QString &strPath)
{
	if (strPath.isEmpty()) {
#if QT_VERSION < 0x050000
		m_strLastPath = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
#else
		m_strLastPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
#endif
	} else {
		QFileInfo fiPath(strPath);
		m_strLastPath = fiPath.absolutePath();
	}
}

QString CSaveLoadFileDialog::lastPath()
{
	return m_strLastPath;
}

// ============================================================================
