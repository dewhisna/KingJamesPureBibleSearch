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

#include "Translator.h"

#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>
#include <QCoreApplication>

#include <assert.h>

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
		m_locale((strLangName.isEmpty()) ? QLocale::system() : strLangName),
		m_bLoaded(false)
{
	m_bLoaded = m_translatorApp.load(m_locale, strTranslationFilename, g_strTranslationFilenamePrefix, g_strTranslationsPath, constrTranslationFilenameSuffix);
	if (m_bLoaded) {
		if (!m_translatorQt.load(m_locale, "qt", "_", g_strTranslationsPath, constrTranslationFilenameSuffix)) {
			// Note: Qt5 is moving toward using individual component translations.  They have the primary qt file, like
			//		"qt_de.qm", but it doesn't have any data.  Instead, it delegates to "qtbase_de.qm", for example.
			//		So if the primary doesn't load, try the base.  This of course makes the deployment more difficult:
			m_translatorQt.load(m_locale, "qtbase", "_", g_strTranslationsPath, constrTranslationFilenameSuffix);
		}
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
	assert(!g_strTranslationsPath.isEmpty());			// Must set global translations path before creating this singleton!
	assert(!g_strTranslationFilenamePrefix.isEmpty());	// Must set global translations filename prefix before creating this singleton!

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
		QCoreApplication::removeTranslator(&m_pCurrentTranslator->translatorApp());
		QCoreApplication::removeTranslator(&m_pCurrentTranslator->translatorQt());
	}

	if (strLangName.isEmpty()) {
		m_pCurrentTranslator = m_pSystemLocaleTranslator;
	} else {
		m_pCurrentTranslator = m_mapTranslators.value(strLangName.toLower(), TTranslatorPtr());
	}

	if (m_pCurrentTranslator.isNull()) return false;

	QCoreApplication::installTranslator(&m_pCurrentTranslator->translatorQt());
	QCoreApplication::installTranslator(&m_pCurrentTranslator->translatorApp());
	return true;
}

QList<CTranslatorList::TLanguageName> CTranslatorList::languageList() const
{
	QList<TLanguageName> lstLanguages;

	lstLanguages.reserve(m_mapTranslators.size());
	for (TTranslatorMap::const_iterator itr = m_mapTranslators.constBegin(); itr != m_mapTranslators.constEnd(); ++itr) {
		if (itr.value().isNull()) continue;
		if (!itr.value()->isLoaded()) continue;
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
