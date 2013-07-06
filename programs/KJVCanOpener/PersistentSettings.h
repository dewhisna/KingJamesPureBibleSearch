/****************************************************************************
**
** Copyright (C) 2012 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef PERSISTENTSETTINGS_H
#define PERSISTENTSETTINGS_H

#include <QObject>
#include <QSettings>
#include <QFont>
#include <QColor>
#include <QList>

extern QString groupCombine(const QString &strSubgroup, const QString &strGroup);

typedef QMap<int, QColor> TUserDefinedColorMap;

class CPersistentSettings : public QObject
{
	Q_OBJECT
private:				// Enforce Singleton:
	CPersistentSettings(QObject *parent = 0);

public:
	~CPersistentSettings();
	static CPersistentSettings *instance();
	inline QSettings &settings() { return *m_pSettings; }

	const QFont &fontScriptureBrowser() const { return m_pPersistentSettingData->m_fntScriptureBrowser; }
	const QFont &fontSearchResults() const { return m_pPersistentSettingData->m_fntSearchResults; }

	bool invertTextBrightness() const { return m_pPersistentSettingData->m_bInvertTextBrightness; }
	int textBrightness() const { return m_pPersistentSettingData->m_nTextBrightness; }
	bool adjustDialogElementBrightness() const { return m_pPersistentSettingData->m_bAdjustDialogElementBrightness; }

	static QColor textForegroundColor(bool bInvert, int nBrightness);
	static QColor textBackgroundColor(bool bInvert, int nBrightness);

	QColor textForegroundColor() const;
	QColor textBackgroundColor() const;

	inline QColor colorWordsOfJesus() const { return m_pPersistentSettingData->m_clrWordsOfJesus; }
	inline QColor colorSearchResults() const { return m_pPersistentSettingData->m_clrSearchResults; }
	inline QColor colorCursorFollow() const { return m_pPersistentSettingData->m_clrCursorFollow; }
	QColor userDefinedColor(int nIndex) const { return m_pPersistentSettingData->m_mapUserHighlighters.value(nIndex, QColor()); }
	bool existsUserDefinedColor(int nIndex) const { return (m_pPersistentSettingData->m_mapUserHighlighters.find(nIndex) != m_pPersistentSettingData->m_mapUserHighlighters.constEnd()); }
	inline const TUserDefinedColorMap &userDefinedColorMap() const { return m_pPersistentSettingData->m_mapUserHighlighters; }

	void togglePersistentSettingData(bool bCopy);

signals:
	void fontChangedScriptureBrowser(const QFont &aFont);
	void fontChangedSearchResults(const QFont &aFont);

	void invertTextBrightnessChanged(bool bInvert);
	void textBrightnessChanged(int nBrightness);
	void adjustDialogElementBrightnessChanged(bool bAdjust);

	void changedTextBrightness(bool bInvert, int nBrightness);

	void changedColorWordsOfJesus(const QColor &color);
	void changedColorSearchResults(const QColor &color);
	void changedColorCursorFollow(const QColor &color);
	void changedUserDefinedColor(int nIndex, const QColor &color);		// Note: If entire map is swapped or cleared, this signal isn't fired!
	void removedUserDefinedColor(int nIndex);							// Note: If entire map is swapped or cleared, this signal isn't fired!
	void changedUserDefinedColors();									// Fired on both individual and entire UserDefinedColor map change

public slots:
	void setFontScriptureBrowser(const QFont &aFont);
	void setFontSearchResults(const QFont &aFont);

	void setInvertTextBrightness(bool bInvert);
	void setTextBrightness(int nBrightness);
	void setAdjustDialogElementBrightness(bool bAdjust);

	void setColorWordsOfJesus(const QColor &color);
	void setColorSearchResults(const QColor &color);
	void setColorCursorFollow(const QColor &color);
	void setUserDefinedColor(int nIndex, const QColor &color);
	void removeUserDefinedColor(int nIndex);
	void removeAllUserDefinedColors();

private:
	// m_PersistentSettingData1 and m_PersistentSettingData2 are
	//		two complete copies of our persistent setting data.  Only
	//		one will be active and used at any given time.  Classes
	//		like KJVConfiguration can request that the settings be
	//		copied to the other copy and that other copy made to be
	//		the main copy for preview purposes so that controls will
	//		appear with the new set of settings.  When it's done with
	//		it, it can either revert back to the original copy without
	//		copying it back or leave the new settings to be the new
	//		settings.
	class TPersistentSettingData {
	public:
		TPersistentSettingData();

		QFont m_fntScriptureBrowser;
		QFont m_fntSearchResults;
		bool m_bInvertTextBrightness;
		int m_nTextBrightness;
		bool m_bAdjustDialogElementBrightness;
		QColor m_clrWordsOfJesus;						// Color for the Words of Jesus (usually "red")
		QColor m_clrSearchResults;						// Color for the Search Results text we find (usually "blue")
		QColor m_clrCursorFollow;						// Color for the CursorFollow underline highlighter (usually "blue")
		TUserDefinedColorMap m_mapUserHighlighters;		// Map of user defined color highlighters
	} m_PersistentSettingData1, m_PersistentSettingData2, *m_pPersistentSettingData;

	QSettings *m_pSettings;
};

#endif // PERSISTENTSETTINGS_H
