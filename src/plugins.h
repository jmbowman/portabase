/*
 * plugins.h
 *
 * (c) 2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file plugins.h
 * Correct handling of Qt plugins when PortaBase is statically linked to Qt.
 */

#ifndef PLUGINS_H
#define PLUGINS_H

#if STATIC_QT

#include <QtPlugin>

Q_IMPORT_PLUGIN(qtaccessiblewidgets)
Q_IMPORT_PLUGIN(qjpeg)
 
#endif
#endif
