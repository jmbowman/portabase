/*
 * commandline.h
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef COMMANDLINE_H
#define COMMANDLINE_H

class CommandLine
{
public:
    CommandLine();
    ~CommandLine();

    int process(int argc, char **argv);

private:
    int fromOtherFormat(int argc, char **argv);
    int toXML(int argc, char **argv);
    void printUsage();
};

#endif
