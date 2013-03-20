#!/bin/sh

#****************************************************************************
#**
#** Copyright (C) 2012 Donna Whisnant, a.k.a. Dewtronics.
#** Contact: http://www.dewtronics.com/
#**
#** This file is part of the KJVCanOpener Application as originally written
#** and developed for Bethel Church, Festus, MO.
#**
#** GNU General Public License Usage
#** This file may be used under the terms of the GNU General Public License
#** version 3.0 as published by the Free Software Foundation and appearing
#** in the file gpl-3.0.txt included in the packaging of this file. Please
#** review the following information to ensure the GNU General Public License
#** version 3.0 requirements will be met:
#** http://www.gnu.org/copyleft/gpl.html.
#**
#** Other Usage
#** Alternatively, this file may be used in accordance with the terms and
#** conditions contained in a signed written agreement between you and
#** Dewtronics.
#**
#****************************************************************************

echo "Installation is complete!"
echo -n "Run King James Pure Bible Search now? (y/n): "
read answer
answer=`echo ${answer} | cut -c1`
echo
if [ "${answer}" = "y" -o "${answer}" = "Y" ]; then
    nohup /bin/sh -c /opt/kjvpurebiblesearch/KJVCanOpener/app/KingJamesPureBibleSearch </dev/null >/dev/null 2>/dev/null &
fi

