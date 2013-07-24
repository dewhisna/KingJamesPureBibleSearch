//
// C++ Implementation: %{MODULE}
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "wwinterfaces.h"
#include <QSettings>

wwWidgetInterface::wwWidgetInterface(QObject *parent) : QObject(parent) {
    m_initialized = false;
    setGroup("wwWidgets");

}

wwWidgetInterface::~ wwWidgetInterface() {}

bool wwWidgetInterface::isContainer() const {
    return false;
}

QString wwWidgetInterface::group() const {
    QSettings settings("wysota.eu.org", "wwWidgetsPlugin");
    int pol = settings.value("GroupPolicy", 0).toInt();
    switch(pol){
        case 1: return m_defGroup;
        case 2: return "wwWidgets";
        case 0: default: return m_group;
    }
}

QIcon wwWidgetInterface::icon() const {
    return QIcon();
}

bool wwWidgetInterface::isInitialized() const {
    return m_initialized;
}

void wwWidgetInterface::initialize(QDesignerFormEditorInterface * core) {
    m_initialized = true;
}

QString wwWidgetInterface::whatsThis() const {
    return QString("%1 - part of wwWidgets widget collection").arg(name());
}

QString wwWidgetInterface::toolTip() const {
    return m_toolTip;
}

QString wwWidgetInterface::name() const {
    QString cl = metaObject()->className();
    cl.chop(5);
    return cl;
}

QString wwWidgetInterface::includeFile() const {
    return name().toLower()+".h";
}



#if QT_VERSION >= 0x040300
wwSheetExtension::wwSheetExtension(QExtensionManager *manager, const QStringList &l, QDesignerPropertySheetExtension *oth,
                                   QDesignerDynamicPropertySheetExtension *othDyn,
                                   QObject *parent)
        : QObject(parent), QDesignerPropertySheetExtension(),
        QDesignerDynamicPropertySheetExtension() {
        m_manager = manager;
    m_other = oth;
    m_otherDynamic = othDyn;
    m_names = l;
    m_beginCount = 0;
    if (m_other)
        m_beginCount = m_other->count();
    if (m_otherDynamic)
        while (m_otherDynamic->isDynamicProperty(m_beginCount - 1))
            m_beginCount--;
}


#else
wwSheetExtension::wwSheetExtension(QExtensionManager *manager, const QStringList &l, QDesignerPropertySheetExtension *oth, QObject * parent) : QObject(parent), QDesignerPropertySheetExtension() {
    m_manager = manager;
    m_other = oth;
    m_names = l;
}
#endif

int wwSheetExtension::count() const {
    int c = m_names.count();
    return other() ? other()->count() + c : c;
}

QDesignerPropertySheetExtension * wwSheetExtension::other() const {
    return m_other;
}

const QStringList & wwSheetExtension::names() const {
    return m_names;
}

QString wwSheetExtension::propertyGroup(int index) const {
//     int myindex = other() ? other()->count() : 0;
//     if (index>=myindex && index<myindex+names().count()) return name();
//     return other() ? other()->propertyGroup(index) : "";
    if (index >= beginCount() && index < beginCount() + names().count())
        return name();
    int newIndex = index < beginCount() ? index : index - names().count();
    return other() ? other()->propertyGroup(newIndex) : QString();
}

QString wwSheetExtension::name() const {
    QString n = metaObject()->className();
    n.chop(14);
    return n;
}

QString wwSheetExtension::propertyName(int index) const {
//     int myindex = other() ? other()->count() : 0;
//     if (index>=myindex) return names().at(index-myindex);
//     return other() ? other()->propertyName(index) : QString::null;
if (index >= beginCount() && index < beginCount() + names().count())
        return names().at(index - beginCount());
    int newIndex = index < beginCount() ? index : index - names().count();
    return other() ? other()->propertyName(newIndex) : QString();
}

int wwSheetExtension::indexOf(const QString & name) const {
//     int myindex = other() ? other()->count() : 0;
//     int i = names().indexOf(name);
//     if (i>=0) return myindex+i;
//     return other() ? other()->indexOf(name) : -1;
    int idx = names().indexOf(name);
    if (idx >= 0)
        return beginCount() + idx;

    if (!other())
        return -1;

    idx = other()->indexOf(name);
    if (idx < beginCount())
        return idx;
    return idx + names().count();
}

bool wwSheetExtension::isVisible(int index) const {
//     int myindex = other() ? other()->count() : 0;
//     if (index>=myindex) return true;
//     return other() ? other()->isVisible(index) : false;
    if (index >= beginCount() && index < beginCount() + names().count())
        return true;
    int newIndex = index < beginCount() ? index : index - names().count();
    return other() ? other()->isVisible(newIndex) : false;
}

bool wwSheetExtension::isAttribute(int index) const {
    if (index >= beginCount() && index < beginCount() + names().count())
        return true;
    int newIndex = index < beginCount() ? index : index - names().count();
    return other() ? other()->isAttribute(newIndex) : false;
}

bool wwSheetExtension::isChanged(int index) const
{
if (index >= beginCount() && index < beginCount() + names().count()) {
        return false;
    }
    if (!other())
        return false;

    int newIndex = index < beginCount() ? index : index - names().count();
    return other()->isChanged(newIndex);
}

QExtensionManager * wwSheetExtension::manager() const
{return m_manager;
}

QVariant wwSheetExtension::property(int index) const
{
    if (index >= beginCount() && index < beginCount() + names().count()) {
        return QVariant();
    }
    int newIndex = index < beginCount() ? index : index - names().count();
    return other() ? other()->property(newIndex) : QVariant();
}

bool wwSheetExtension::hasReset(int index) const
{
    if (index >= beginCount() && index < beginCount() + names().count()) {

        return false;
    }
    if (!other())
        return false;

    int newIndex = index < beginCount() ? index : index - names().count();
    return other()->hasReset(newIndex);
}

bool wwSheetExtension::reset(int index)
{
if (index >= beginCount() && index < beginCount() + names().count()) {
        return false;
    }
    if (!other())
        return false;

    int newIndex = index < beginCount() ? index : index - names().count();
    return other()->reset(newIndex);
}

void wwSheetExtension::setProperty(int index, const QVariant & value)
{
if (index >= beginCount() && index < beginCount() + names().count()) {
        return;
    }
    if (!other())
        return;

    int newIndex = index < beginCount() ? index : index - names().count();
    other()->setProperty(newIndex, value);
}

void wwSheetExtension::setChanged(int index, bool changed)
{
if (index >= beginCount() && index < beginCount() + names().count()) {
        return;
    }
    if (!other())
        return;

    int newIndex = index < beginCount() ? index : index - names().count();
    other()->setChanged(newIndex, changed);
}

void wwWidgetInterface::setGroup(const QString &grp)
{
    m_group = grp;
}

void wwWidgetInterface::setDefaultGroup(const QString &grp)
{
    m_defGroup = grp;
}

void wwWidgetInterface::setToolTip(const QString &tt)
{
    m_toolTip = tt;
}

#if QT_VERSION >= 0x040300
QDesignerDynamicPropertySheetExtension* wwSheetExtension::otherDynamic() const {
    return m_otherDynamic;
}

int wwSheetExtension::beginCount() const {
    return m_beginCount;
}

int wwSheetExtension::addDynamicProperty ( const QString & propertyName, const QVariant & value ) {
    return otherDynamic() ? otherDynamic()->addDynamicProperty(propertyName, value) : -1;
}

bool wwSheetExtension::canAddDynamicProperty ( const QString & propertyName ) const {
    if (!otherDynamic())
        return false;

    if (names().indexOf(propertyName) >= 0)
        return false;

    return otherDynamic()->canAddDynamicProperty(propertyName);
}

bool wwSheetExtension::dynamicPropertiesAllowed () const {
    return otherDynamic() ? otherDynamic()->dynamicPropertiesAllowed() : false;
}

bool wwSheetExtension::isDynamicProperty ( int index ) const {
    int newIndex = index < beginCount() ? index : index - names().count();
    return otherDynamic() ? otherDynamic()->isDynamicProperty(newIndex) : false;
}

bool wwSheetExtension::removeDynamicProperty ( int index ) {
    int newIndex = index < beginCount() ? index : index - names().count();
    return otherDynamic() ? otherDynamic()->removeDynamicProperty(newIndex) : false;
}
#endif
