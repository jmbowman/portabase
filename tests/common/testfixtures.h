/*
 * testfixtures.h
 *
 * (c) 2026 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * Shared builder for structurally valid PortaBase test fixture files.
 * Owned by neither the column-editing nor the integrity-check segment;
 * see docs/intent/column-editing/column-editing-design.md § Test Fixtures
 * Must Be Structurally Valid Files.
 */

#ifndef TESTFIXTURES_H
#define TESTFIXTURES_H

#include <QString>

namespace PbTest {

QString buildStandardFile(const QString &path, int dataRows = 2);

QStringList standardColumnNames();

}

#endif
