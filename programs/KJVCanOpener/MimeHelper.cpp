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

#include "MimeHelper.h"

#include <QApplication>
#include <QMimeData>
#include <QClipboard>
#include <QDataStream>
#include <QByteArray>

#include <assert.h>

CMimeHelper::CMimeHelper()
{

}

void CMimeHelper::addPhraseTagToClipboard(const TPhraseTag &tag)
{
	QClipboard *clipboard = QApplication::clipboard();
	const QMimeData *pClipboardMime = clipboard->mimeData();
	QMimeData *mime = new QMimeData();
	if (pClipboardMime->hasText()) mime->setText(pClipboardMime->text());
	if (pClipboardMime->hasHtml()) mime->setHtml(pClipboardMime->html());
	addPhraseTagToMimeData(mime, tag);
	clipboard->setMimeData(mime);
}

void CMimeHelper::addPhraseTagToMimeData(QMimeData *pMime, const TPhraseTag &tag)
{
	assert(pMime != NULL);
	QByteArray arrData;
	QDataStream ds(&arrData, QIODevice::WriteOnly);
	ds << tag;
	pMime->setData(g_constrPhraseTagMimeType, arrData);
}

TPhraseTag CMimeHelper::getPhraseTagFromMimeData(const QMimeData *pMime)
{
	assert(pMime != NULL);
	QByteArray arrData(pMime->data(g_constrPhraseTagMimeType));
	QDataStream ds(arrData);
	TPhraseTag tag;
	ds >> tag;
	return tag;
}

