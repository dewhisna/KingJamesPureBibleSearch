//
// C++ Interface: wwinterfaces
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef __WWINTERFACES_H
#define __WWINTERFACES_H

#include <QDesignerCustomWidgetInterface>
#include <QVariant>

#include <QtDesigner/QDesignerPropertySheetExtension>
#include <QtDesigner/QDesignerContainerExtension>
#if QT_VERSION >= 0x040300
#include <QtDesigner/QDesignerDynamicPropertySheetExtension>
#endif

/**
 *  @internal
 */
class wwWidgetInterface : public QObject, public QDesignerCustomWidgetInterface {
public:
    wwWidgetInterface(QObject *parent=0);
    ~wwWidgetInterface();
    bool isContainer() const;
    bool isInitialized() const;
    QIcon icon() const;
    QString name() const;
    QString group() const;
    QString toolTip() const;
    QString whatsThis() const;
    QString includeFile() const;
    void initialize(QDesignerFormEditorInterface *core);
protected:
    void setGroup(const QString &);
    void setDefaultGroup(const QString &);
    void setToolTip(const QString &);
private:
    bool m_initialized;
    QString m_group;
    QString m_defGroup;
    QString m_toolTip;
};

class QExtensionManager;
/**
 *  @internal
 */
class wwSheetExtension : public QObject, public QDesignerPropertySheetExtension
#if QT_VERSION >= 0x040300
, public QDesignerDynamicPropertySheetExtension
#endif
 {
public:
#if QT_VERSION >= 0x040300
    wwSheetExtension(QExtensionManager *manager, const QStringList &l, QDesignerPropertySheetExtension *oth,
                     QDesignerDynamicPropertySheetExtension *othDyn, QObject *parent=0);
#else
    wwSheetExtension(QExtensionManager *manager, const QStringList &l, QDesignerPropertySheetExtension *oth, QObject *parent=0);
#endif
    int count() const;
    QVariant property(int index) const;
    QString propertyGroup(int index) const;
    QString propertyName ( int index ) const;
    int indexOf ( const QString & name ) const;
    bool isVisible ( int index ) const;
    virtual QWidget *currentPage() const = 0;
    QExtensionManager *manager() const;
    bool isChanged(int index) const;
    bool isAttribute ( int index ) const;
    bool hasReset(int index) const;
    bool reset(int index);
    void setProperty(int index, const QVariant & value);
    void setChanged(int index, bool changed);
#if QT_VERSION >= 0x040300
    int beginCount() const;
    int addDynamicProperty ( const QString & propertyName, const QVariant & value );
    bool canAddDynamicProperty ( const QString & propertyName ) const;
    bool dynamicPropertiesAllowed () const;
    bool isDynamicProperty ( int index ) const;
    bool removeDynamicProperty ( int index );
#endif
protected:
    QDesignerPropertySheetExtension * other() const;
#if QT_VERSION >= 0x040300
    QDesignerDynamicPropertySheetExtension *otherDynamic() const;
#endif
    const QStringList &names() const;
    QString name() const;
private:
    QExtensionManager *m_manager;
    QDesignerPropertySheetExtension *m_other;
#if QT_VERSION >= 0x040300
    QDesignerDynamicPropertySheetExtension *m_otherDynamic;
    int m_beginCount;
#endif
    QStringList m_names;
};

#endif
