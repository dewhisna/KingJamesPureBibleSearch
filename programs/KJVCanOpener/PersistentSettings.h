#ifndef PERSISTENTSETTINGS_H
#define PERSISTENTSETTINGS_H

#include <QObject>
#include <QSettings>


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

signals:

public slots:


private:
	QSettings *m_pSettings;
};

#endif // PERSISTENTSETTINGS_H
