//
// Bible Text Database Interface
//

#include "BibleText.h"

#include <dbDescriptors.h>
#include <ReadDB.h>

#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>

// ============================================================================

namespace {
	const char *g_constrBibleDatabasePath = "../../KJVCanOpener/db/";

	const BIBLE_DESCRIPTOR_ENUM g_SPMT_BibleDescriptor = BDE_SPMT_20120627;
}

// ============================================================================

MasoreticPentateuch::MasoreticPentateuch()
{
}

const CBibleDatabase *MasoreticPentateuch::instance()
{
	static CBibleDatabasePtr spmt;
	if (!spmt.isNull()) return spmt.data();

	static const TBibleDescriptor &spmtBibleDescriptor = bibleDescriptor(g_SPMT_BibleDescriptor);

	CReadDatabase rdbSPMT(databasePath(), QString(), nullptr);
	if ((rdbSPMT.haveBibleDatabaseFiles(spmtBibleDescriptor)) &&
		(rdbSPMT.ReadBibleDatabase(spmtBibleDescriptor))) {
		spmt = TBibleDatabaseList::instance()->atUUID(spmtBibleDescriptor.m_strUUID);
	}

	return spmt.data();
}

QString MasoreticPentateuch::databasePath()
{
	QFileInfo fiDBPath(QDir(QCoreApplication::applicationDirPath()), g_constrBibleDatabasePath);
	return fiDBPath.absoluteFilePath();
}
