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

#ifndef SAVE_FILE_DIALOG_H
#define SAVE_FILE_DIALOG_H

#include <QString>
#include <QWidget>
#include <QFileDialog>
#include <QPointer>

// ============================================================================

class CSaveFileDialog
{
public:
	static QString getSaveFileName(QWidget *parent = NULL,
												const QString &caption = QString(),
												const QString &dir = QString(),
												const QString &filter = QString(),
												const QString &strDefaultSuffix = QString(),
												QString *selectedFilter = NULL,
												QFileDialog::Options options = 0)
	{
		QPointer<QFileDialog> pDlg = new QFileDialog(parent, caption, dir, filter);
		pDlg->setFileMode(QFileDialog::AnyFile);
		pDlg->setAcceptMode(QFileDialog::AcceptSave);
		pDlg->setDefaultSuffix(strDefaultSuffix);
		pDlg->setOptions(options);
		if (selectedFilter) pDlg->selectNameFilter(*selectedFilter);
		if (pDlg->exec() == QDialog::Accepted) {
			if (pDlg.data() != NULL) {
				return pDlg->selectedFiles().value(0);
			}
		}
		if (pDlg) delete pDlg;

		return QString();
	}
};

// ============================================================================

#endif		// SAVE_FILE_DIALOG_H
