/*******************************************************
**
** Implementation of Q4puGenericSignalSpy
**
** Copyright (C) 2005 Prashanth N Udupa
**
** Q4puGenericSignalSpy can be used to spy on QObjects
** for signals that they emit, and the slots that are
** invoked in them.
**
********************************************************/

/***************************************************************************
                          Q4puGenericSignalSpy.cpp
                             -------------------
    begin                : Mon 8 August 2005
    copyright            : (C) 2005 by Prashanth Udupa
    email                : prashanth.udupa@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "Q4puGenericSignalSpy.h"
#include <QMetaObject>
#include <QMetaMethod>

/**
\class Q4puGenericSignalSpy Q4puGenericSignalSpy.h
\brief Spies for signals and slots in an object

Q4puGenericSignalSpy is a Qt4 avtar of QpuGenericSignalSpy written
for Qt3. This class took lesser time to implement because of Qt4.
The meta object system in Qt4 has received a good overhaul and the
changes are very nice. Infact the new meta object system is easier
to use than the previous ones.

\section sec1 What does this class do ?
This class looks for signals and slots emitted from a QObject in
a Qt4 application. Whenever the object that is being spied on 
emits a signal or has one of its slots invoked; this class emits
a signal informing the world about it.

Take a look at the example below
\code
int main(int argc, char **argv)
{
	QApplication a(argc, argv);
	QWidget * topLevel = new QWidget;
	QVBoxLayout * layout = new QVBoxLayout(topLevel);
	QDirModel * dirModel = new QDirModel(topLevel);
	QTreeView * treeView = new QTreeView(topLevel);
	treeView->setObjectName("DirTreeView");
	layout->addWidget(treeView);
	treeView->setModel(dirModel);
	Q3TextEdit * textEdit = new Q3TextEdit(topLevel);
		layout->addWidget(textEdit);
	Q4puGenericSignalSpy * spy = new Q4puGenericSignalSpy(&a);
	spy->spyOn(treeView);
	QObject::connect(spy, SIGNAL(caughtSignal(const QString&)),
					 textEdit, SLOT(append(const QString&)));
	QObject::connect(spy, SIGNAL(caughtSlot(const QString&)),
					 textEdit, SLOT(append(const QString&)));
	topLevel->show();
	return a.exec();
}
\endcode

When executed the following output was observed
\image html q4pugenericsignalspy.jpg

*/

void q4pugss_registerSpy(Q4puGenericSignalSpy* spy);
void q4pugss_unregisterSpy(Q4puGenericSignalSpy* spy);

/**
\brief Constructor
*/
Q4puGenericSignalSpy::Q4puGenericSignalSpy(QObject* parent)
					 :QObject(parent)
{
	q4pugss_registerSpy(this);
}

/**
\brief Destructor
*/
Q4puGenericSignalSpy::~Q4puGenericSignalSpy()
{
	q4pugss_unregisterSpy(this);
}

/**
\fn void Q4puGenericSignalSpy::spyOn(QObject* object)
\brief Specifies the object to spy on
*/

/**
\fn QObject* Q4puGenericSignalSpy::spyingOn() const
\brief Returns the object that is currently being spied on
*/


/**
\defgroup q4pugss
\internal
\brief Helper functions for Q4puGenericSignalSpy class

The \ref QSignalSpyCallbackSet structure in Qt4 consists of
four function pointers. These functions are call back functions
that will be called whenever the Qt4 runtime system invokes
a slot or emits a signal.

The \ref qt_signal_spy_callback_set variable is of type 
\ref QSignalSpyCallbackSet and is defined in qcoreapplication.cpp.
Since the symbol is exported, we can gain access to this variable.
If we set the four function pointers in this variable to point
to callbacks in this module; we have effectively asked Qt4 runtime
to inform us
1. Before a slot is invoked
2. After an invoked slot has returned
3. Before a signal is emitted
4. After a signal has been emitted
*/


void q4pugss_BeginCallBackSignal(QObject* caller, int method_index, void **argv);
void q4pugss_EndCallBackSignal(QObject* caller, int method_index);
void q4pugss_BeginCallBackSlot(QObject* caller, int method_index, void **argv);
void q4pugss_EndCallBackSlot(QObject* caller, int method_index);
bool q4pugss_GetMethodString(QObject* caller, int method_index, void **argv, 
	 QString &string);
const QString q4pugss_value(const char *type, void *value);
static QList<Q4puGenericSignalSpy*> SpyList;
static uint TabSize = 0;

/////////////========== Copied from qobject_p.h
// qobject_p.h is not a public Qt4 API. Hence 
// #include <qobject_p.h> wont work. 
// So we have copied some lines from qobject_p.h 
// in here to serve the purpose.

/**
\ingroup q4pugss
\internal
\class QSignalSpyCallbackSet
*/
struct QSignalSpyCallbackSet
{
	/**
	\brief Function type for a begin callback function
	*/
    typedef void (*BeginCallback)(QObject *caller, int method_index, void **argv);
    
	/**
	\brief Function type for an end callback function
	*/
	typedef void (*EndCallback)(QObject *caller, int method_index);
	
	/**
	\brief Function pointer to the function that is called before a
	signal is emitted
	*/
    BeginCallback signal_begin_callback;
	/**
	\brief Function pointer to the function that is called before a
	slot is invoked
	*/
	BeginCallback slot_begin_callback;
	/**
	\brief Function pointer to the function that is called after a 
	signal emission is complete
	*/
    EndCallback signal_end_callback;
	/**
	\brief Function pointer to the function that is called after a
	invoked slot has returned
	*/
	EndCallback slot_end_callback;
};

/**
\ingroup q4pugss
\internal
\brief Declaration of the function we should call to register our
call back structure. Implementation of this function can be found in
qcoreapplication.cpp
*/
void Q_CORE_EXPORT qt_register_signal_spy_callbacks(const QSignalSpyCallbackSet &callback_set);

/**
\ingroup q4pugss
\internal 
\brief The \ref QSignalSpyCallbackSet variable. The Qt4 runtime system
calls functions in this variable at appropriate times
*/
extern QSignalSpyCallbackSet Q_CORE_EXPORT qt_signal_spy_callback_set;
/////////////========== 

/**
\ingroup q4pugss
\internal
\brief 
*/
void q4pugss_registerCallBacks()
{
	QSignalSpyCallbackSet cb;
	cb.signal_begin_callback = q4pugss_BeginCallBackSignal;
	cb.signal_end_callback   = q4pugss_EndCallBackSignal;
	cb.slot_begin_callback   = q4pugss_BeginCallBackSlot;
	cb.slot_end_callback     = q4pugss_EndCallBackSlot;
	qt_register_signal_spy_callbacks(cb);
}

/**
\ingroup q4pugss
\internal
\brief 
*/
void q4pugss_registerSpy(Q4puGenericSignalSpy* spy)
{
	if( !SpyList.count() )
		q4pugss_registerCallBacks();
	if( SpyList.indexOf(spy) == -1 )
		SpyList.append(spy);
}

/**
\ingroup q4pugss
\internal
\brief 
*/
void q4pugss_unregisterSpy(Q4puGenericSignalSpy* spy)
{
	int index;
	if( (index=SpyList.indexOf(spy)) == -1 )
		return;
	SpyList.removeAt(index);
}

/**
\ingroup q4pugss
\internal
\brief 
*/
void q4pugss_BeginCallBackSignal(QObject* caller, int method_index, void **argv)
{
	QString sig_param;
	for(int i=0; i<SpyList.count(); i++) {
		Q4puGenericSignalSpy * spy = SpyList[i];
		if( spy->spyingOn() == caller )
			if(q4pugss_GetMethodString(caller, method_index, argv, sig_param))
				spy->emitCaughtSignal(sig_param);
	}
	TabSize += 4;
}

/**
\ingroup q4pugss
\internal
\brief 
*/
void q4pugss_EndCallBackSignal(QObject*, int)
{
	// do nothing
	TabSize -= 4;
}

/**
\ingroup q4pugss
\internal
\brief 
*/
void q4pugss_BeginCallBackSlot(QObject* caller, int method_index, void **argv)
{
	QString sig_param;
	for(int i=0; i<SpyList.count(); i++) {
		Q4puGenericSignalSpy * spy = SpyList[i];
		if( spy->spyingOn() == caller ) 
			if(q4pugss_GetMethodString(caller, method_index, argv, sig_param))
				spy->emitCaughtSlot(sig_param);
	}
}

/**
\ingroup q4pugss
\internal
\brief 
*/
void q4pugss_EndCallBackSlot(QObject*, int)
{
	// do nothing
}

/**
\ingroup q4pugss
\internal
\brief 
*/
bool q4pugss_GetMethodString(QObject* caller, int method_index, void **argv,  QString &string)
{
	const QMetaObject * mo = caller->metaObject();
	if( !mo )
		return false;
	QMetaMethod m = mo->method(method_index);
	if( method_index >= mo->methodCount() )
		return false;
	
	static QString methodType[] = {"Method", "Signal", "Slot"};
	
	string.fill(' ', TabSize);
	string = QString("%1 (%2) ")
				.arg(caller->objectName().isNull()?"noname":caller->objectName())
				.arg(mo->className());
	string += QString("%1: %2(")
				.arg(methodType[(int)m.methodType()])
				.arg(QString(m.signature()).section('(',0,0));
	
	QList<QByteArray> pNames = m.parameterNames();
	QList<QByteArray> pTypes = m.parameterTypes();
	
	for(int i=0; i<pNames.count(); i++) {
		string += QString("%1=%2")
					.arg(QString(pNames.at(i)))
					.arg(q4pugss_value(pTypes.at(i), argv[i+1]));
		if(i != pNames.count()-1)
			string += ", ";
	}
	
	string += QString(")");
	
	return true;
}

/**
\ingroup q4pugss
\internal
\brief 
*/
const QString q4pugss_value(const char *type, void *argv)
{
	QVariant v( QVariant::nameToType(type), argv );
	if( v.type() )
		return QString("%1(%2)").arg(type).arg(v.toString());
	return QString("%1 <cannot decode>").arg(type);
}

