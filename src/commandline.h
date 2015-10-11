/*
 * commandline.h
 *
 * (c) 2003,2008,2013,2015 by Jeremy Bowman <jmbowman@alum.mit.edu>
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
    int parseOption(const QString &option, bool takesArgument=true);
    static void printUsage();
    static QString validateEncoding(int argIndex);

private:
    int argc; /**< The total number of arguments that were provided */
    int numArgs; /**< The number of recognized arguments found so far */
};

#endif
