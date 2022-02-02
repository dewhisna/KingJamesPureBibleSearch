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

#include "Translator.h"

#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>
#include <QCoreApplication>

// ============================================================================

namespace {
	//////////////////////////////////////////////////////////////////////
	// File-scoped constants
	//////////////////////////////////////////////////////////////////////

	const char *constrTranslationFilenameSuffix = ".qm";

}	// namespace

// ============================================================================

// Globals:

QString g_strTranslationsPath;
QString g_strTranslationFilenamePrefix;

// ============================================================================

CTranslator::CTranslator(const QString &strLangName, const QString &strTranslationFilename, QObject *pParent)
	:	QObject(pParent),
		m_strLangName(strLangName),
		m_locale((strLangName.isEmpty()) ? QLocale::system() : QLocale(strLangName)),
		m_bLoadedApp(false),
		m_bLoadedwwWidgets(false),
		m_bLoadedQt(false)
{
	m_bLoadedApp = m_translatorApp.load(m_locale, strTranslationFilename, g_strTranslationFilenamePrefix, g_strTranslationsPath, constrTranslationFilenameSuffix);
	if (m_bLoadedApp) {
		m_bLoadedwwWidgets = m_translatorwwWidgets.load(m_locale, "wwwidgets", "_", g_strTranslationsPath, constrTranslationFilenameSuffix);

		// Note: Qt5 is moving toward using individual component translations with a main qt_XX.qm file that delegates
		//		to the submodules.  However, they have only done this migration for DE, not ES or FR.  Also, we don't
		//		really want to include all of the submodule translations since we aren't even using them (why ship
		//		them), but if we don't, then the qt_XX.qm doesn't load.  Instead, we have to load qtbase_XX.qm.
		//		BUT, it's much more worse than that!  They've apparently changed the translation namespace for the
		//		dialog buttons from QDialogButtonBox to QPlatformTheme.  This means that the individual qt_XX.qm
		//		translation files won't even work on Qt5, which is all they have for ES and FR...
		// SO... I have hacked the qt_XX verions for ES and FR to the correct namespace and made them qtbase_XX to
		//		match the new Qt5 naming convention for use with Qt5.  And we have the old qt_XX for the Qt4 builds.
		//		That means that here we must load qt_XX.qm for Qt4 and qtbase_XX.qm for Qt5...  (ugh!)
#if QT_VERSION < 0x050000
		m_bLoadedQt = m_translatorQt.load(m_locale, "qt", "_", g_strTranslationsPath, constrTranslationFilenameSuffix);
#else
		m_bLoadedQt = m_translatorQt.load(m_locale, "qtbase", "_", g_strTranslationsPath, constrTranslationFilenameSuffix);
#endif
	}
}

CTranslator::~CTranslator()
{

}

QString CTranslator::name(bool bLangOnly) const
{
	QString strName = m_locale.name();
	if (bLangOnly) {
		int nPos = strName.indexOf(QChar('_'));
		if (nPos != -1) strName = strName.left(nPos);
		strName = strName.toLower();
	}
	return strName;
}

QString CTranslator::nativeLanguageName() const
{
	switch (m_locale.language()) {
		case QLocale::English:
			return tr("English", "languageNames");
		case QLocale::French:
			return tr("French", "languageNames");
		case QLocale::Spanish:
			return tr("Spanish", "languageNames");
		case QLocale::German:
			return tr("German", "languageNames");
		default:
			return QLocale::languageToString(m_locale.language());
//			return m_locale.nativeLanguageName();
	}
}

// ============================================================================

CTranslatorList::CTranslatorList(QObject *pParent)
	:	QObject(pParent)
{
	Q_ASSERT(!g_strTranslationsPath.isEmpty());			// Must set global translations path before creating this singleton!
	Q_ASSERT(!g_strTranslationFilenamePrefix.isEmpty());	// Must set global translations filename prefix before creating this singleton!

	QDir dirTranslations(g_strTranslationsPath, g_strTranslationFilenamePrefix + ".*" + QString(constrTranslationFilenameSuffix));
	QFileInfoList lstTranslationFiles = dirTranslations.entryInfoList();
	for (int ndx = 0; ndx < lstTranslationFiles.size(); ++ndx) {
		const QFileInfo &fiTranslationFile = lstTranslationFiles.at(ndx);
		if (!fiTranslationFile.exists() || !fiTranslationFile.isFile()) continue;
		QString strBaseName = fiTranslationFile.completeBaseName();
		int nIDIndex = strBaseName.indexOf(QChar('.'));
		if (nIDIndex == -1) continue;
		QString strLanguageID = strBaseName.mid(nIDIndex+1).toLower();
		TTranslatorPtr pTranslator = new CTranslator(strLanguageID, strBaseName, this);
		m_mapTranslators[strLanguageID] = pTranslator;
	}

	m_pSystemLocaleTranslator = new CTranslator(QString(), QString(), this);
}

CTranslatorList::~CTranslatorList()
{

}

CTranslatorList *CTranslatorList::instance()
{
	static CTranslatorList theTranslatorList;
	return &theTranslatorList;
}

bool CTranslatorList::setApplicationLanguage(const QString &strLangName)
{
	if (!m_pCurrentTranslator.isNull()) {
		if (m_pCurrentTranslator->isLoadedApp())
			QCoreApplication::removeTranslator(&m_pCurrentTranslator->translatorApp());
		if (m_pCurrentTranslator->isLoadedwwWidgets())
			QCoreApplication::removeTranslator(&m_pCurrentTranslator->translatorwwWidgets());
		if (m_pCurrentTranslator->isLoadedQt())
			QCoreApplication::removeTranslator(&m_pCurrentTranslator->translatorQt());
	}

	if (strLangName.isEmpty()) {
		m_pCurrentTranslator = m_pSystemLocaleTranslator;
	} else {
		m_pCurrentTranslator = m_mapTranslators.value(strLangName.toLower(), TTranslatorPtr());
	}

	if (m_pCurrentTranslator.isNull()) return false;

	if (m_pCurrentTranslator->isLoadedQt())
		QCoreApplication::installTranslator(&m_pCurrentTranslator->translatorQt());
	if (m_pCurrentTranslator->isLoadedwwWidgets())
		QCoreApplication::installTranslator(&m_pCurrentTranslator->translatorwwWidgets());
	if (m_pCurrentTranslator->isLoadedApp())
		QCoreApplication::installTranslator(&m_pCurrentTranslator->translatorApp());
	return true;
}

QList<CTranslatorList::TLanguageName> CTranslatorList::languageList() const
{
	QList<TLanguageName> lstLanguages;

	lstLanguages.reserve(m_mapTranslators.size());
	for (TTranslatorMap::const_iterator itr = m_mapTranslators.constBegin(); itr != m_mapTranslators.constEnd(); ++itr) {
		if (itr.value().isNull()) continue;
		if (!itr.value()->isLoadedApp()) continue;
		lstLanguages.append(TLanguageName(itr.key(), itr.value()->nativeLanguageName()));
	}

	return lstLanguages;
}

TTranslatorPtr CTranslatorList::translator(const QString &strLangName) const
{
	if (strLangName.isEmpty()) return m_pSystemLocaleTranslator;
	return m_mapTranslators.value(strLangName.toLower(), TTranslatorPtr());
}

// ============================================================================
