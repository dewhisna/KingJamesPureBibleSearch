/****************************************************************************
**
** Copyright (C) 2012 BogDan Vatra <bogdan@kde.org>
** Contact: http://www.qt-project.org/legal
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qandroidassetsfileenginehandler.h"
#include <android/asset_manager_jni.h>
#include <android/log.h>

#include <QtAndroidExtras/QAndroidJniObject>
#include <QtAndroidExtras/QAndroidJniEnvironment>

#include <QCoreApplication>

static const char g_kjpbsTag[] = "KJPBS";
static const char g_classErrorMsg[] = "Can't find class \"%s\"";
static const char g_methodErrorMsg[] = "Can't find method \"%s%s\"";
static jclass g_applicationClass  = NULL;
static AAssetManager *g_assetManager = NULL;
QString AndroidAssetsFileEngineHandler::g_strPackageCodePath;

static AAssetManager *assetManager()
{
	return g_assetManager;
}

#define FIND_AND_CHECK_CLASS(CLASS_NAME) \
clazz = env->FindClass(CLASS_NAME); \
if (!clazz) { \
	__android_log_print(ANDROID_LOG_FATAL, g_kjpbsTag, g_classErrorMsg, CLASS_NAME); \
	return JNI_FALSE; \
}

#define GET_AND_CHECK_METHOD(VAR, CLASS, METHOD_NAME, METHOD_SIGNATURE) \
VAR = env->GetMethodID(CLASS, METHOD_NAME, METHOD_SIGNATURE); \
if (!VAR) { \
	__android_log_print(ANDROID_LOG_FATAL, g_kjpbsTag, g_methodErrorMsg, METHOD_NAME, METHOD_SIGNATURE); \
	return JNI_FALSE; \
}

#define GET_AND_CHECK_STATIC_METHOD(VAR, CLASS, METHOD_NAME, METHOD_SIGNATURE) \
VAR = env->GetStaticMethodID(CLASS, METHOD_NAME, METHOD_SIGNATURE); \
if (!VAR) { \
	__android_log_print(ANDROID_LOG_FATAL, g_kjpbsTag, g_methodErrorMsg, METHOD_NAME, METHOD_SIGNATURE); \
	return JNI_FALSE; \
}

#define GET_AND_CHECK_FIELD(VAR, CLASS, FIELD_NAME, FIELD_SIGNATURE) \
VAR = env->GetFieldID(CLASS, FIELD_NAME, FIELD_SIGNATURE); \
if (!VAR) { \
	__android_log_print(ANDROID_LOG_FATAL, g_kjpbsTag, g_methodErrorMsg, FIELD_NAME, FIELD_SIGNATURE); \
	return JNI_FALSE; \
}

#define GET_AND_CHECK_STATIC_FIELD(VAR, CLASS, FIELD_NAME, FIELD_SIGNATURE) \
VAR = env->GetStaticFieldID(CLASS, FIELD_NAME, FIELD_SIGNATURE); \
if (!VAR) { \
	__android_log_print(ANDROID_LOG_FATAL, g_kjpbsTag, g_methodErrorMsg, FIELD_NAME, FIELD_SIGNATURE); \
	return JNI_FALSE; \
}

int AndroidAssetsFileEngineHandler::registerNatives(JNIEnv *env)
{
	__android_log_print(ANDROID_LOG_INFO, g_kjpbsTag, "in registerNatives");

	if (env == NULL) return JNI_FALSE;

	if (g_assetManager == NULL) {
		jclass clazz;
		FIND_AND_CHECK_CLASS("org/qtproject/qt5/android/QtNative");
		g_applicationClass = static_cast<jclass>(env->NewGlobalRef(clazz));

		jmethodID methodID;

		GET_AND_CHECK_STATIC_METHOD(methodID, g_applicationClass, "activity", "()Landroid/app/Activity;");
		jobject objActivity = env->CallStaticObjectMethod(g_applicationClass, methodID);

		clazz = env->GetObjectClass(objActivity);

		GET_AND_CHECK_METHOD(methodID, clazz, "getPackageCodePath", "()Ljava/lang/String;");
		jobject objPackageCodePath = env->CallObjectMethod(objActivity, methodID);

		const char *jstrPackageCodePath;
		jboolean isCopy = JNI_FALSE;
		jstrPackageCodePath = env->GetStringUTFChars((jstring) objPackageCodePath, &isCopy);
		g_strPackageCodePath = QString::fromUtf8(jstrPackageCodePath);
		__android_log_print(ANDROID_LOG_INFO, g_kjpbsTag, QString("PackageCodePath: %1").arg(AndroidAssetsFileEngineHandler::packageCodePath()).toUtf8().data());


		FIND_AND_CHECK_CLASS("android/content/ContextWrapper");
		GET_AND_CHECK_METHOD(methodID, clazz, "getAssets", "()Landroid/content/res/AssetManager;");

		g_assetManager = AAssetManager_fromJava(env, env->CallObjectMethod(objActivity, methodID));

		if (g_assetManager == NULL) {
			__android_log_print(ANDROID_LOG_FATAL, g_kjpbsTag, "assetManager_fromJava returned NULL");
		}


////		QAndroidJniObject
////		GET_AND_CHECK_STATIC_METHOD(methodID, m_applicationClass, "activity", "()Landroid/app/Activity;");
////	    jobject activityObject = env->CallStaticObjectMethod(m_applicationClass, methodID);
//
//		QAndroidJniObject activityObject("android/app/Activity");
//		jclass clazz = env->FindClass("android/content/ContextWrapper");
//		methodID = env->GetMethodID(clazz, "getAssets", "()Landroid/content/res/AssetManager;");
//		g_assetManager = AAssetManager_fromJava(env, env->CallObjectMethod(activityObject.object<jobject>(), methodID));
	}

	__android_log_print(ANDROID_LOG_INFO, g_kjpbsTag, "registerNatives complete");

	return JNI_TRUE;
}

Q_DECL_EXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void */*reserved*/)
{
	typedef union {
		JNIEnv *nativeEnvironment;
		void *venv;
	} UnionJNIEnvToVoid;

	UnionJNIEnvToVoid uenv;
	uenv.venv = NULL;

	if ((vm == NULL) ||
		(vm->GetEnv(&uenv.venv, JNI_VERSION_1_4) != JNI_OK)) return -1;

	JNIEnv *env = uenv.nativeEnvironment;
	if (!AndroidAssetsFileEngineHandler::registerNatives(env)) return -1;

	return JNI_VERSION_1_4;
}

class AndroidAbstractFileEngineIterator: public QAbstractFileEngineIterator
{
public:
    AndroidAbstractFileEngineIterator(QDir::Filters filters,
                                      const QStringList &nameFilters,
                                      AAssetDir *asset,
                                      const QString &path)
        : QAbstractFileEngineIterator(filters, nameFilters)
    {
        AAssetDir_rewind(asset);
        const char *fileName;
        while ((fileName = AAssetDir_getNextFileName(asset)))
            m_items << fileName;
        m_index = -1;
        m_path = path;
    }

    virtual QFileInfo currentFileInfo() const
    {
        return QFileInfo(currentFilePath());
    }

    virtual QString currentFileName() const
    {
        if (m_index < 0 || m_index >= m_items.size())
            return QString();
        return m_items[m_index];
    }

    virtual QString currentFilePath() const
    {
        return m_path + currentFileName();
    }

    virtual bool hasNext() const
    {
        return m_items.size() && (m_index < m_items.size() - 1);
    }

    virtual QString next()
    {
        if (!hasNext())
            return QString();
        m_index++;
        return currentFileName();
    }

private:
    QString     m_path;
    QStringList m_items;
    int         m_index;
};

class AndroidAbstractFileEngine: public QAbstractFileEngine
{
public:
    explicit AndroidAbstractFileEngine(AAsset *asset, const QString &fileName)
    {
        m_assetDir = 0;
        m_assetFile = asset;
        m_fileName = fileName;
    }

    explicit AndroidAbstractFileEngine(AAssetDir *asset, const QString &fileName)
    {
        m_assetFile = 0;
        m_assetDir = asset;
        m_fileName =  fileName;
        if (!m_fileName.endsWith(QChar(QLatin1Char('/'))))
            m_fileName += "/";
    }

    ~AndroidAbstractFileEngine()
    {
        close();
        if (m_assetDir)
            AAssetDir_close(m_assetDir);
    }

    virtual bool open(QIODevice::OpenMode openMode)
    {
        if (m_assetFile)
            return openMode & QIODevice::ReadOnly;
        return false;
    }

    virtual bool close()
    {
        if (m_assetFile) {
            AAsset_close(m_assetFile);
            m_assetFile = 0;
            return true;
        }
        return false;
    }

    virtual qint64 size() const
    {
        if (m_assetFile)
            return AAsset_getLength(m_assetFile);
        return -1;
    }

    virtual qint64 pos() const
    {
        if (m_assetFile)
            return AAsset_seek(m_assetFile, 0, SEEK_CUR);
        return -1;
    }

    virtual bool seek(qint64 pos)
    {
        if (m_assetFile)
            return pos == AAsset_seek(m_assetFile, pos, SEEK_SET);
        return false;
    }

    virtual qint64 read(char *data, qint64 maxlen)
    {
        if (m_assetFile)
            return AAsset_read(m_assetFile, data, maxlen);
        return -1;
    }

    virtual bool isSequential() const
    {
        return false;
    }

    virtual bool caseSensitive() const
    {
        return true;
    }

    virtual bool isRelativePath() const
    {
        return false;
    }

    virtual FileFlags fileFlags(FileFlags type = FileInfoAll) const
    {
        FileFlags flags(ReadOwnerPerm|ReadUserPerm|ReadGroupPerm|ReadOtherPerm|ExistsFlag);
        if (m_assetFile)
            flags |= FileType;
        if (m_assetDir)
            flags |= DirectoryType;

        return type & flags;
    }

    virtual QString fileName(FileName file = DefaultName) const
    {
        int pos;
        switch (file) {
        case DefaultName:
        case AbsoluteName:
        case CanonicalName:
                return m_fileName;
        case BaseName:
            if ((pos = m_fileName.lastIndexOf(QChar(QLatin1Char('/')))) != -1)
                return m_fileName.mid(pos);
            else
                return m_fileName;
        case PathName:
        case AbsolutePathName:
        case CanonicalPathName:
            if ((pos = m_fileName.lastIndexOf(QChar(QLatin1Char('/')))) != -1)
                return m_fileName.left(pos);
            else
                return m_fileName;
        default:
            return QString();
        }
    }

    virtual void setFileName(const QString &file)
    {
        if (file == m_fileName)
            return;

        m_fileName = file;
        if (!m_fileName.endsWith(QChar(QLatin1Char('/'))))
            m_fileName += "/";

        close();
    }

    virtual Iterator *beginEntryList(QDir::Filters filters, const QStringList &filterNames)
    {
        if (m_assetDir)
            return new AndroidAbstractFileEngineIterator(filters, filterNames, m_assetDir, m_fileName);
		return NULL;
    }

private:
    AAsset *m_assetFile;
    AAssetDir *m_assetDir;
    QString m_fileName;
};


AndroidAssetsFileEngineHandler::AndroidAssetsFileEngineHandler()
{
	m_assetManager = assetManager();
}

AndroidAssetsFileEngineHandler::~AndroidAssetsFileEngineHandler()
{
}

QString AndroidAssetsFileEngineHandler::packageCodePath()
{
	return g_strPackageCodePath;
}

QAbstractFileEngine * AndroidAssetsFileEngineHandler::create(const QString &fileName) const
{
	__android_log_print(ANDROID_LOG_INFO, g_kjpbsTag, QString("In AndroidAssetsFileEngineHandler::create for \"%1\"").arg(fileName).toUtf8().data());
	if (m_assetManager == NULL) {
		__android_log_print(ANDROID_LOG_FATAL, g_kjpbsTag, "AndroidAssetsFileEngineHandler::create - assetManager is NULL");
		return NULL;
	}

	if (fileName.isEmpty()) {
		__android_log_print(ANDROID_LOG_WARN, g_kjpbsTag, "AndroidAssetsFileEngineHandler::create - fileName is empty");
		return NULL;
	}

	if (!fileName.startsWith(QLatin1String("assets:/"))) {
		__android_log_print(ANDROID_LOG_WARN, g_kjpbsTag, "AndroidAssetsFileEngineHandler::create - fileName not in \"assets:/\" namespace");
		return NULL;
	}

    int prefixSize=8;

    m_path.clear();
    if (!fileName.endsWith(QLatin1Char('/'))) {
        m_path = fileName.toUtf8();
        AAsset *asset = AAssetManager_open(m_assetManager,
                                           m_path.constData() + prefixSize,
                                           AASSET_MODE_BUFFER);
		if (asset) {
			__android_log_print(ANDROID_LOG_INFO, g_kjpbsTag, QString("AndroidAssetsFileEngineHandler::create - Open File : \"%1\" SUCCESS").arg(fileName).toUtf8().data());
            return new AndroidAbstractFileEngine(asset, fileName);
		} else {
			__android_log_print(ANDROID_LOG_WARN, g_kjpbsTag, QString("AndroidAssetsFileEngineHandler::create - open \"%1\" as file failed").arg(fileName).toUtf8().data());
		}
    }

	if (!m_path.size())
		 m_path = fileName.left(fileName.length() - 1).toUtf8();

    AAssetDir *assetDir = AAssetManager_openDir(m_assetManager, m_path.constData() + prefixSize);
    if (assetDir) {
		__android_log_print(ANDROID_LOG_INFO, g_kjpbsTag, QString("AndroidAssetsFileEngineHandler::create - Open Dir : \"%1\" SUCCESS").arg(fileName).toUtf8().data());
		if (AAssetDir_getNextFileName(assetDir)) {
			__android_log_print(ANDROID_LOG_INFO, g_kjpbsTag, "GetNextFileName for Dir SUCCESS");
            return new AndroidAbstractFileEngine(assetDir, fileName);
		} else {
			__android_log_print(ANDROID_LOG_INFO, g_kjpbsTag, "GetNextFileName for Dir AT END");
            AAssetDir_close(assetDir);
		}
	} else {
		__android_log_print(ANDROID_LOG_WARN, g_kjpbsTag, QString("AndroidAssetsFileEngineHandler::create - open \"%1\" as dir failed").arg(fileName).toUtf8().data());
	}

	return NULL;
}
