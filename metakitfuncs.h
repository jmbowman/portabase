/*
 * metakitfuncs.h
 *
 * (c) 2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qstring.h>

int compareStrings(const QString &s1, const QString &s2);
int compareUsingLocale(const char *s1, const char *s2);
int windowsFileOpen(const char *filename, int flags);
