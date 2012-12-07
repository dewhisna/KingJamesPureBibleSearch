#ifndef MIMEHELPER_H
#define MIMEHELPER_H

#include "dbstruct.h"

#include <QMimeData>

class CMimeHelper
{
public:
	CMimeHelper();

	static void addPhraseTagToClipboard(const TPhraseTag &tag);
	static void addPhraseTagToMimeData(QMimeData *pMime, const TPhraseTag &tag);
	static TPhraseTag getPhraseTagFromMimeData(const QMimeData *pMime);
};

#endif // MIMEHELPER_H
