/*
 * pbfileselector.cpp
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "fileselector.h"

PBFileSelector::PBFileSelector(const QString &f, QWidget *parent, const char *name)
  : FileSelector(f, parent, name, FALSE, FALSE)
{

}

PBFileSelector::~PBFileSelector()
{

}
