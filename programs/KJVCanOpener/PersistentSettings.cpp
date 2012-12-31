#include "PersistentSettings.h"

#include "version.h"

#include <QCoreApplication>

#include <assert.h>

// ============================================================================

namespace
{
	//////////////////////////////////////////////////////////////////////
	// File-scoped constants
	//////////////////////////////////////////////////////////////////////

	// Key constants:
	// --------------

}

// ============================================================================

CPersistentSettings::CPersistentSettings(QObject *parent)
	:	QObject(parent)
{
	// Must set these in main() before caling settings!:
	assert(QCoreApplication::applicationName().compare(VER_APPNAME_STR_QT) == 0);
	assert(QCoreApplication::organizationName().compare(VER_ORGNAME_STR_QT) == 0);
	assert(QCoreApplication::organizationDomain().compare(VER_ORGDOMAIN_STR_QT) == 0);

	m_pSettings = new QSettings();
}

CPersistentSettings::~CPersistentSettings()
{
	delete m_pSettings;
}

CPersistentSettings *CPersistentSettings::instance()
{
	static CPersistentSettings thePersistentSettings;
	return &thePersistentSettings;
}

