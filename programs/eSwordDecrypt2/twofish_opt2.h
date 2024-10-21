/****************************************************************************
**
** Copyright (C) 2024 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef TWOFISH_OPT2_H_
#define TWOFISH_OPT2_H_

#include <sstream>
#include <string>
#include <stdint.h>

// ============================================================================

class CSQLitePlusDecryptor
{
public:
	CSQLitePlusDecryptor(const std::string &strCryptKey);

	int decrypt(std::stringstream &ssSource, std::string &strPlain);

	bool test();

private:
	uint32_t K[40];
	uint32_t QF[4][256];
};

// ============================================================================

#endif	// TWOFISH_OPT2_H_
