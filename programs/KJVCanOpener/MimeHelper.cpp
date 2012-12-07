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

