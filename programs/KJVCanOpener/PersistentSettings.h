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

extern QString groupCombine(const QString &strSubgroup, const QString &strGroup);


class CPersistentSettings : public QObject
{
	Q_OBJECT
private:				// Enforce Singleton:
	CPersistentSettings(QObject *parent = 0);

public:
	~CPersistentSettings();
	static CPersistentSettings *instance();
	inline QSettings &settings() { return *m_pSettings; }

	const QFont &fontScriptureBrowser() const { return m_fntScriptureBrowser; }
	const QFont &fontSearchResults() const { return m_fntSearchResults; }

	bool invertTextBrightness() const { return m_bInvertTextBrightness; }
	int textBrightness() const { return m_nTextBrightness; }
	bool adjustDialogElementBrightness() const { return m_bAdjustDialogElementBrightness; }

	static QColor textForegroundColor(bool bInvert, int nBrightness);
	static QColor textBackgroundColor(bool bInvert, int nBrightness);

	QColor textForegroundColor() const;
	QColor textBackgroundColor() const;

signals:
	void fontChangedScriptureBrowser(const QFont &aFont);
	void fontChangedSearchResults(const QFont &aFont);

	void invertTextBrightnessChanged(bool bInvert);
	void textBrightnessChanged(int nBrightness);
	void adjustDialogElementBrightnessChanged(bool bAdjust);

	void changedTextBrightness(bool bInvert, int nBrightness);

public slots:
	void setFontScriptureBrowser(const QFont &aFont);
	void setFontSearchResults(const QFont &aFont);

	void setInvertTextBrightness(bool bInvert);
	void setTextBrightness(int nBrightness);
	void setAdjustDialogElementBrightness(bool bAdjust);

private:
	QFont m_fntScriptureBrowser;
	QFont m_fntSearchResults;
	bool m_bInvertTextBrightness;
	int m_nTextBrightness;
	bool m_bAdjustDialogElementBrightness;
	QSettings *m_pSettings;
};

#endif // PERSISTENTSETTINGS_H
