/****************************************************************************
**
** Copyright (C) 2024-2025 Donna Whisnant, a.k.a. Dewtronics.
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

#include <stdint.h>
typedef uint8_t u8;

u8 RS[4][8] = {
    { 0x01, 0xA4, 0x55, 0x87, 0x5A, 0x58, 0xDB, 0x9E, },
    { 0xA4, 0x56, 0x82, 0xF3, 0x1E, 0xC6, 0x68, 0xE5, },
    { 0x02, 0xA1, 0xFC, 0xC1, 0x47, 0xAE, 0x3D, 0x19, },
    { 0xA4, 0x55, 0x87, 0x5A, 0x58, 0xDB, 0x9E, 0x03, },
};

u8 Q0[] = {
    0xA9, 0x67, 0xB3, 0xE8, 0x04, 0xFD, 0xA3, 0x76, 
    0x9A, 0x92, 0x80, 0x78, 0xE4, 0xDD, 0xD1, 0x38, 
    0x0D, 0xC6, 0x35, 0x98, 0x18, 0xF7, 0xEC, 0x6C, 
    0x43, 0x75, 0x37, 0x26, 0xFA, 0x13, 0x94, 0x48, 
    0xF2, 0xD0, 0x8B, 0x30, 0x84, 0x54, 0xDF, 0x23, 
    0x19, 0x5B, 0x3D, 0x59, 0xF3, 0xAE, 0xA2, 0x82, 
    0x63, 0x01, 0x83, 0x2E, 0xD9, 0x51, 0x9B, 0x7C, 
    0xA6, 0xEB, 0xA5, 0xBE, 0x16, 0x0C, 0xE3, 0x61, 
    0xC0, 0x8C, 0x3A, 0xF5, 0x73, 0x2C, 0x25, 0x0B, 
    0xBB, 0x4E, 0x89, 0x6B, 0x53, 0x6A, 0xB4, 0xF1, 
    0xE1, 0xE6, 0xBD, 0x45, 0xE2, 0xF4, 0xB6, 0x66, 
    0xCC, 0x95, 0x03, 0x56, 0xD4, 0x1C, 0x1E, 0xD7, 
    0xFB, 0xC3, 0x8E, 0xB5, 0xE9, 0xCF, 0xBF, 0xBA, 
    0xEA, 0x77, 0x39, 0xAF, 0x33, 0xC9, 0x62, 0x71, 
    0x81, 0x79, 0x09, 0xAD, 0x24, 0xCD, 0xF9, 0xD8, 
    0xE5, 0xC5, 0xB9, 0x4D, 0x44, 0x08, 0x86, 0xE7, 
    0xA1, 0x1D, 0xAA, 0xED, 0x06, 0x70, 0xB2, 0xD2, 
    0x41, 0x7B, 0xA0, 0x11, 0x31, 0xC2, 0x27, 0x90, 
    0x20, 0xF6, 0x60, 0xFF, 0x96, 0x5C, 0xB1, 0xAB, 
    0x9E, 0x9C, 0x52, 0x1B, 0x5F, 0x93, 0x0A, 0xEF, 
    0x91, 0x85, 0x49, 0xEE, 0x2D, 0x4F, 0x8F, 0x3B, 
    0x47, 0x87, 0x6D, 0x46, 0xD6, 0x3E, 0x69, 0x64, 
    0x2A, 0xCE, 0xCB, 0x2F, 0xFC, 0x97, 0x05, 0x7A, 
    0xAC, 0x7F, 0xD5, 0x1A, 0x4B, 0x0E, 0xA7, 0x5A, 
    0x28, 0x14, 0x3F, 0x29, 0x88, 0x3C, 0x4C, 0x02, 
    0xB8, 0xDA, 0xB0, 0x17, 0x55, 0x1F, 0x8A, 0x7D, 
    0x57, 0xC7, 0x8D, 0x74, 0xB7, 0xC4, 0x9F, 0x72, 
    0x7E, 0x15, 0x22, 0x12, 0x58, 0x07, 0x99, 0x34, 
    0x6E, 0x50, 0xDE, 0x68, 0x65, 0xBC, 0xDB, 0xF8, 
    0xC8, 0xA8, 0x2B, 0x40, 0xDC, 0xFE, 0x32, 0xA4, 
    0xCA, 0x10, 0x21, 0xF0, 0xD3, 0x5D, 0x0F, 0x00, 
    0x6F, 0x9D, 0x36, 0x42, 0x4A, 0x5E, 0xC1, 0xE0, 
    };

u8 Q1[] = {
    0x75, 0xF3, 0xC6, 0xF4, 0xDB, 0x7B, 0xFB, 0xC8, 
    0x4A, 0xD3, 0xE6, 0x6B, 0x45, 0x7D, 0xE8, 0x4B, 
    0xD6, 0x32, 0xD8, 0xFD, 0x37, 0x71, 0xF1, 0xE1, 
    0x30, 0x0F, 0xF8, 0x1B, 0x87, 0xFA, 0x06, 0x3F, 
    0x5E, 0xBA, 0xAE, 0x5B, 0x8A, 0x00, 0xBC, 0x9D, 
    0x6D, 0xC1, 0xB1, 0x0E, 0x80, 0x5D, 0xD2, 0xD5, 
    0xA0, 0x84, 0x07, 0x14, 0xB5, 0x90, 0x2C, 0xA3, 
    0xB2, 0x73, 0x4C, 0x54, 0x92, 0x74, 0x36, 0x51, 
    0x38, 0xB0, 0xBD, 0x5A, 0xFC, 0x60, 0x62, 0x96, 
    0x6C, 0x42, 0xF7, 0x10, 0x7C, 0x28, 0x27, 0x8C, 
    0x13, 0x95, 0x9C, 0xC7, 0x24, 0x46, 0x3B, 0x70, 
    0xCA, 0xE3, 0x85, 0xCB, 0x11, 0xD0, 0x93, 0xB8, 
    0xA6, 0x83, 0x20, 0xFF, 0x9F, 0x77, 0xC3, 0xCC, 
    0x03, 0x6F, 0x08, 0xBF, 0x40, 0xE7, 0x2B, 0xE2, 
    0x79, 0x0C, 0xAA, 0x82, 0x41, 0x3A, 0xEA, 0xB9, 
    0xE4, 0x9A, 0xA4, 0x97, 0x7E, 0xDA, 0x7A, 0x17, 
    0x66, 0x94, 0xA1, 0x1D, 0x3D, 0xF0, 0xDE, 0xB3, 
    0x0B, 0x72, 0xA7, 0x1C, 0xEF, 0xD1, 0x53, 0x3E, 
    0x8F, 0x33, 0x26, 0x5F, 0xEC, 0x76, 0x2A, 0x49, 
    0x81, 0x88, 0xEE, 0x21, 0xC4, 0x1A, 0xEB, 0xD9, 
    0xC5, 0x39, 0x99, 0xCD, 0xAD, 0x31, 0x8B, 0x01, 
    0x18, 0x23, 0xDD, 0x1F, 0x4E, 0x2D, 0xF9, 0x48, 
    0x4F, 0xF2, 0x65, 0x8E, 0x78, 0x5C, 0x58, 0x19, 
    0x8D, 0xE5, 0x98, 0x57, 0x67, 0x7F, 0x05, 0x64, 
    0xAF, 0x63, 0xB6, 0xFE, 0xF5, 0xB7, 0x3C, 0xA5, 
    0xCE, 0xE9, 0x68, 0x44, 0xE0, 0x4D, 0x43, 0x69, 
    0x29, 0x2E, 0xAC, 0x15, 0x59, 0xA8, 0x0A, 0x9E, 
    0x6E, 0x47, 0xDF, 0x34, 0x35, 0x6A, 0xCF, 0xDC, 
    0x22, 0xC9, 0xC0, 0x9B, 0x89, 0xD4, 0xED, 0xAB, 
    0x12, 0xA2, 0x0D, 0x52, 0xBB, 0x02, 0x2F, 0xA9, 
    0xD7, 0x61, 0x1E, 0xB4, 0x50, 0x04, 0xF6, 0xC2, 
    0x16, 0x25, 0x86, 0x56, 0x55, 0x09, 0xBE, 0x91, 
    };

u8 mult5B[] = {
    0x00, 0x5B, 0xB6, 0xED, 0x05, 0x5E, 0xB3, 0xE8, 
    0x0A, 0x51, 0xBC, 0xE7, 0x0F, 0x54, 0xB9, 0xE2, 
    0x14, 0x4F, 0xA2, 0xF9, 0x11, 0x4A, 0xA7, 0xFC, 
    0x1E, 0x45, 0xA8, 0xF3, 0x1B, 0x40, 0xAD, 0xF6, 
    0x28, 0x73, 0x9E, 0xC5, 0x2D, 0x76, 0x9B, 0xC0, 
    0x22, 0x79, 0x94, 0xCF, 0x27, 0x7C, 0x91, 0xCA, 
    0x3C, 0x67, 0x8A, 0xD1, 0x39, 0x62, 0x8F, 0xD4, 
    0x36, 0x6D, 0x80, 0xDB, 0x33, 0x68, 0x85, 0xDE, 
    0x50, 0x0B, 0xE6, 0xBD, 0x55, 0x0E, 0xE3, 0xB8, 
    0x5A, 0x01, 0xEC, 0xB7, 0x5F, 0x04, 0xE9, 0xB2, 
    0x44, 0x1F, 0xF2, 0xA9, 0x41, 0x1A, 0xF7, 0xAC, 
    0x4E, 0x15, 0xF8, 0xA3, 0x4B, 0x10, 0xFD, 0xA6, 
    0x78, 0x23, 0xCE, 0x95, 0x7D, 0x26, 0xCB, 0x90, 
    0x72, 0x29, 0xC4, 0x9F, 0x77, 0x2C, 0xC1, 0x9A, 
    0x6C, 0x37, 0xDA, 0x81, 0x69, 0x32, 0xDF, 0x84, 
    0x66, 0x3D, 0xD0, 0x8B, 0x63, 0x38, 0xD5, 0x8E, 
    0xA0, 0xFB, 0x16, 0x4D, 0xA5, 0xFE, 0x13, 0x48, 
    0xAA, 0xF1, 0x1C, 0x47, 0xAF, 0xF4, 0x19, 0x42, 
    0xB4, 0xEF, 0x02, 0x59, 0xB1, 0xEA, 0x07, 0x5C, 
    0xBE, 0xE5, 0x08, 0x53, 0xBB, 0xE0, 0x0D, 0x56, 
    0x88, 0xD3, 0x3E, 0x65, 0x8D, 0xD6, 0x3B, 0x60, 
    0x82, 0xD9, 0x34, 0x6F, 0x87, 0xDC, 0x31, 0x6A, 
    0x9C, 0xC7, 0x2A, 0x71, 0x99, 0xC2, 0x2F, 0x74, 
    0x96, 0xCD, 0x20, 0x7B, 0x93, 0xC8, 0x25, 0x7E, 
    0xF0, 0xAB, 0x46, 0x1D, 0xF5, 0xAE, 0x43, 0x18, 
    0xFA, 0xA1, 0x4C, 0x17, 0xFF, 0xA4, 0x49, 0x12, 
    0xE4, 0xBF, 0x52, 0x09, 0xE1, 0xBA, 0x57, 0x0C, 
    0xEE, 0xB5, 0x58, 0x03, 0xEB, 0xB0, 0x5D, 0x06, 
    0xD8, 0x83, 0x6E, 0x35, 0xDD, 0x86, 0x6B, 0x30, 
    0xD2, 0x89, 0x64, 0x3F, 0xD7, 0x8C, 0x61, 0x3A, 
    0xCC, 0x97, 0x7A, 0x21, 0xC9, 0x92, 0x7F, 0x24, 
    0xC6, 0x9D, 0x70, 0x2B, 0xC3, 0x98, 0x75, 0x2E, 
    };

u8 multEF[] = {
    0x00, 0xEF, 0xB7, 0x58, 0x07, 0xE8, 0xB0, 0x5F, 
    0x0E, 0xE1, 0xB9, 0x56, 0x09, 0xE6, 0xBE, 0x51, 
    0x1C, 0xF3, 0xAB, 0x44, 0x1B, 0xF4, 0xAC, 0x43, 
    0x12, 0xFD, 0xA5, 0x4A, 0x15, 0xFA, 0xA2, 0x4D, 
    0x38, 0xD7, 0x8F, 0x60, 0x3F, 0xD0, 0x88, 0x67, 
    0x36, 0xD9, 0x81, 0x6E, 0x31, 0xDE, 0x86, 0x69, 
    0x24, 0xCB, 0x93, 0x7C, 0x23, 0xCC, 0x94, 0x7B, 
    0x2A, 0xC5, 0x9D, 0x72, 0x2D, 0xC2, 0x9A, 0x75, 
    0x70, 0x9F, 0xC7, 0x28, 0x77, 0x98, 0xC0, 0x2F, 
    0x7E, 0x91, 0xC9, 0x26, 0x79, 0x96, 0xCE, 0x21, 
    0x6C, 0x83, 0xDB, 0x34, 0x6B, 0x84, 0xDC, 0x33, 
    0x62, 0x8D, 0xD5, 0x3A, 0x65, 0x8A, 0xD2, 0x3D, 
    0x48, 0xA7, 0xFF, 0x10, 0x4F, 0xA0, 0xF8, 0x17, 
    0x46, 0xA9, 0xF1, 0x1E, 0x41, 0xAE, 0xF6, 0x19, 
    0x54, 0xBB, 0xE3, 0x0C, 0x53, 0xBC, 0xE4, 0x0B, 
    0x5A, 0xB5, 0xED, 0x02, 0x5D, 0xB2, 0xEA, 0x05, 
    0xE0, 0x0F, 0x57, 0xB8, 0xE7, 0x08, 0x50, 0xBF, 
    0xEE, 0x01, 0x59, 0xB6, 0xE9, 0x06, 0x5E, 0xB1, 
    0xFC, 0x13, 0x4B, 0xA4, 0xFB, 0x14, 0x4C, 0xA3, 
    0xF2, 0x1D, 0x45, 0xAA, 0xF5, 0x1A, 0x42, 0xAD, 
    0xD8, 0x37, 0x6F, 0x80, 0xDF, 0x30, 0x68, 0x87, 
    0xD6, 0x39, 0x61, 0x8E, 0xD1, 0x3E, 0x66, 0x89, 
    0xC4, 0x2B, 0x73, 0x9C, 0xC3, 0x2C, 0x74, 0x9B, 
    0xCA, 0x25, 0x7D, 0x92, 0xCD, 0x22, 0x7A, 0x95, 
    0x90, 0x7F, 0x27, 0xC8, 0x97, 0x78, 0x20, 0xCF, 
    0x9E, 0x71, 0x29, 0xC6, 0x99, 0x76, 0x2E, 0xC1, 
    0x8C, 0x63, 0x3B, 0xD4, 0x8B, 0x64, 0x3C, 0xD3, 
    0x82, 0x6D, 0x35, 0xDA, 0x85, 0x6A, 0x32, 0xDD, 
    0xA8, 0x47, 0x1F, 0xF0, 0xAF, 0x40, 0x18, 0xF7, 
    0xA6, 0x49, 0x11, 0xFE, 0xA1, 0x4E, 0x16, 0xF9, 
    0xB4, 0x5B, 0x03, 0xEC, 0xB3, 0x5C, 0x04, 0xEB, 
    0xBA, 0x55, 0x0D, 0xE2, 0xBD, 0x52, 0x0A, 0xE5, 
    };

