/****************************************************************************
**
** Copyright (C) 2014-2020 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef SAVE_LOAD_FILE_DIALOG_H
#define SAVE_LOAD_FILE_DIALOG_H

#include <QString>
#include <QWidget>
#include <QFileDialog>

// ============================================================================

class CSaveLoadFileDialog
{
public:
	static QString getSaveFileName(QWidget *parent = NULL,
												const QString &caption = QString(),
												const QString &dir = QString(),
												const QString &filter = QString(),
												const QString &strDefaultSuffix = QString(),
												QString *selectedFilter = NULL,
												QFileDialog::Options options = 0);

	static QString getOpenFileName(QWidget *parent = NULL,
												const QString &caption = QString(),
												const QString &dir = QString(),
												const QString &filter = QString(),
												QString *selectedFilter = NULL,
												QFileDialog::Options options = 0);

protected:
	static void setLastPath(const QString &strPath);
	static QString lastPath();

private:
	static QString m_strLastPath;
};

// ============================================================================

#endif		// SAVE_LOAD_FILE_DIALOG_H
