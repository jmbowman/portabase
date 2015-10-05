/*
 * metakitfuncs.cpp
 *
 * (c) 2004,2008-2009 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file metakitfuncs.cpp
 * Source file for replacement Metakit string comparison function
 */

#include "metakitfuncs.h"

int compareUsingLocale(const char *s1, const char *s2)
{
    QString str1 = QString::fromUtf8(s1);
    QString str2 = QString::fromUtf8(s2);
    return str1.localeAwareCompare(str2);
}
