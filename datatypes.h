/*
 * datatypes.h
 *
 * (c) 2002 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef DATATYPES_H
#define DATATYPES_H

#include <qvaluelist.h>

#define STRING 0
#define INTEGER 1
#define FLOAT 2
#define BOOLEAN 3
#define DATE 4
#define NOTE 5

typedef QValueList<int> IntList;

#endif
