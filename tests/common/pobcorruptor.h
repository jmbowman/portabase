/*
 * pobcorruptor.h
 *
 * (c) 2026 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * Raw-Metakit mutation helpers for seeding structural corruption into
 * PortaBase test fixture files -- the states that decades of real-world
 * file history (and past bugs) can produce but the application API never
 * creates. Used by both the column-editing crash-reproducer tests and the
 * integrity-check tests; owned by neither segment.
 *
 * Every helper opens the file's Metakit storage read-write, applies one
 * mutation, and commits. The file must not be open elsewhere at the time.
 */

#ifndef POBCORRUPTOR_H
#define POBCORRUPTOR_H

#include <QByteArray>
#include <QString>
#include <QStringList>

namespace PobCorrupt {

/** One "where" condition: match rows whose string property equals a value. */
struct Where {
    const char *prop;
    QByteArray value;
    Where(const char *p, const QString &v) : prop(p), value(v.toUtf8()) {}
};

bool setString(const QString &path, const char *viewName, const Where &where,
               const char *prop, const QByteArray &newValue);

bool setString(const QString &path, const char *viewName, const Where &where1,
               const Where &where2, const char *prop,
               const QByteArray &newValue);

bool setInt(const QString &path, const char *viewName, const Where &where,
            const char *prop, int newValue);

bool setInt(const QString &path, const char *viewName, const Where &where1,
            const Where &where2, const char *prop, int newValue);

bool setIntAt(const QString &path, const char *viewName, int rowIndex,
              const char *prop, int newValue);

bool setStringAt(const QString &path, const char *viewName, int rowIndex,
                 const char *prop, const QByteArray &newValue);

bool removeRows(const QString &path, const char *viewName, const Where &where);

bool removeRows(const QString &path, const char *viewName, const Where &where1,
                const Where &where2);

bool duplicateRow(const QString &path, const char *viewName,
                  const Where &where);

bool duplicateRowAt(const QString &path, const char *viewName, int rowIndex);

bool removeRowAt(const QString &path, const char *viewName, int rowIndex);

bool removePortabaseView(const QString &path, const QString &viewName);

bool addCryptoView(const QString &path);

bool createRawFile(const QString &path, int gversion,
                   const QStringList &viewNames, int gcrypt = 0);

}

#endif
