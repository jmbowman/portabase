/*
 * metakitfuncs.cpp
 *
 * (c) 2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qapplication.h>
#include <string.h>
#include "metakitfuncs.h"

#if defined(Q_WS_WIN)
#include <io.h>
#endif

int compareStrings(const QString &s1, const QString &s2)
{
#if QT_VERSION >= 300
    return s1.localeAwareCompare(s2);
#else
    // do the right thing for null and empty
    if (s1.isEmpty() || s2.isEmpty()) {
        return s1.compare(s2);
    }
    // declared in <string.h>
    int delta = strcoll(s1.local8Bit(), s2.local8Bit());
    if (delta == 0) {
        delta = s1.compare(s2);
    }
    return delta;
#endif
}

int compareUsingLocale(const char *s1, const char *s2)
{
    return compareStrings(QString::fromUtf8(s1), QString::fromUtf8(s2));
}

#if defined(Q_WS_WIN)
int windowsFileOpen(const char *filename, int flags)
{
    QString name = QString::fromUtf8(filename);
    if (QApplication::winVersion() & Qt::WV_NT_based) {
        return _wopen(name.ucs2(), flags);
    }
    else {
        return _open(name.latin1(), flags);
    }
}
#else
int windowsFileOpen(const char *, int)
{
    return -1;
}
#endif
