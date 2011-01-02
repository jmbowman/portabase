/*
 * commandline.h
 *
 * (c) 2003,2008 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file commandline.h
 * Header file for CommandLine
 */

#ifndef COMMANDLINE_H
#define COMMANDLINE_H

#include <QStringList>

/**
 * This class manages usage of %PortaBase as a command line tool.  This is
 * typically used for import/export of file contents, often as part of a
 * script.
 */
class CommandLine
{
public:
    CommandLine();

    int process();

private:
    int fromOtherFormat(const QStringList &args);
    int toOtherFormat(const QStringList &args);
    void printUsage();
};

#endif
