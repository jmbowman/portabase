/*
 * datatypes.h
 *
 * (c) 2002,2008,2012,2026 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file datatypes.h
 * Header file containing the column type codes, file format version, and
 * other small shared declarations. Deliberately dependency-light so any
 * component can include it; see docs/wiki/format.md for what the codes mean.
 */

#ifndef DATATYPES_H
#define DATATYPES_H

#include <QList>

#define STRING 0
#define INTEGER 1
#define FLOAT 2
#define BOOLEAN 3
#define NOTE 4
#define DATE 5
#define TIME 6
#define CALC 7
#define SEQUENCE 8
#define IMAGE 9
#define LAST_TYPE 9

#define FIRST_ENUM 100

// newest file format version this build writes and understands
#define FILE_VERSION 11

// default date and time codes
#define TODAY 0
#define NOW 0

typedef QList<int> IntList;

#ifdef TRACE_ENABLED
#define TRACE qDebug("%s@%d", __FILE__, __LINE__)
#else
#define TRACE
#endif

#endif
