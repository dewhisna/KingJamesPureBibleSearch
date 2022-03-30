/****************************************************************************
**
** Copyright (C) 2019-2022 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef QT_QSTYLEOPTION_STUB_H
#define QT_QSTYLEOPTION_STUB_H

#include <QtGlobal>

#if QT_VERSION >= 0x050000
#include <QStyleOptionViewItem>
typedef QStyleOptionViewItem QStyleOptionViewItemV4_t;
typedef QStyleOptionViewItem QStyleOptionViewItemV2_t;

#include <QStyleOptionFrame>
typedef QStyleOptionFrame QStyleOptionFrameV2_t;
#else
#include <QStyleOptionViewItemV4>
typedef QStyleOptionViewItemV4 QStyleOptionViewItemV4_t;
typedef QStyleOptionViewItemV2 QStyleOptionViewItemV2_t;

#include <QStyleOptionFrameV2>
typedef QStyleOptionFrameV2 QStyleOptionFrameV2_t;
#endif

#endif	// QT_QSTYLEOPTION_STUB_H
