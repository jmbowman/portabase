/*
 * commandline.cpp
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qapplication.h>
#include <qfile.h>
#include <qstringlist.h>
#include "commandline.h"
#include "database.h"
#include "importutils.h"
#include "view.h"

CommandLine::CommandLine()
{

}

CommandLine::~CommandLine()
{

}

int CommandLine::process(int argc, char **argv)
{
    if (argv[1] == QCString("fromxml")) {
        return fromOtherFormat(argc, argv);
    }
    else if (argv[1] == QCString("frommobiledb")) {
        return fromOtherFormat(argc, argv);
    }
    else if (argv[1] == QCString("toxml")) {
        return toXML(argc, argv);
    }
    else if (argv[1] == QCString("-h") || argv[1] == QCString("--help")) {
        printUsage();
        return 0;
    }
    else {
        printUsage();
        return 1;
    }
}

int CommandLine::fromOtherFormat(int argc, char **argv)
{
    if (argc < 4) {
        printUsage();
        qApp->exit(1);
        return 1;
    }
    QString sourceFile(argv[2]);
    QString pbFile(argv[3]);
    bool ok;
    Database *db = new Database(pbFile, &ok);
    if (!ok) {
        printf("Unable to create/initialize file\n");
        return 1;
    }
    QString error = "";
    ImportUtils utils;
    if (argv[1] == QCString("fromxml")) {
        error = utils.importXML(sourceFile, db);
    }
    else {
        error = utils.importMobileDB(sourceFile, db);
    }
    if (error != "") {
        printf(error + "\n");
        QFile::remove(pbFile);
        return 1;
    }
    db->commit();
    delete db;
    return 0;
}

int CommandLine::toXML(int argc, char **argv)
{
    QStringList args;
    for (int i = 0; i < argc; i++) {
        args.append(argv[i]);
    }
    int numArgs = 4;
    int viewIndex = args.findIndex("-v");
    if (viewIndex != -1) {
        if (argc < viewIndex + 4) {
            printUsage();
            return 1;
        }
        numArgs += 2;
    }
    int sortIndex = args.findIndex("-s");
    if (args.findIndex("-s") != -1) {
        if (argc < sortIndex + 4) {
            printUsage();
            return 1;
        }
        numArgs += 2;
    }
    int filterIndex = args.findIndex("-f");
    if (args.findIndex("-f") != -1) {
        numArgs += 2;
        if (argc < filterIndex + 4) {
            printUsage();
            return 1;
        }
    }
    if (argc != numArgs) {
        printUsage();
        return 1;
    }
    QString pbFile(args[numArgs - 2]);
    QString xmlFile(args[numArgs - 1]);
    bool ok = FALSE;
    Database *db = new Database(pbFile, &ok);
    if (!ok) {
        printf("Error opening file: %s\n", argv[numArgs - 2]);
        return 1;
    }
    QString viewName = db->currentView();
    QString sortName = db->currentSorting();
    QString filterName = db->currentFilter();
    if (viewIndex != -1) {
        viewName = args[viewIndex + 1];
        if (db->listViews().findIndex(viewName) == -1) {
            printf("Unknown view: %s\n", argv[viewIndex + 1]);
            return 1;
        }
    }
    if (sortIndex != -1) {
        sortName = args[sortIndex + 1];
        if (db->listSortings().findIndex(sortName) == -1) {
            printf("Unknown sorting: %s\n", argv[sortIndex + 1]);
            return 1;
        }
    }
    if (filterIndex != -1) {
        filterName = args[filterIndex + 1];
        if (db->listFilters().findIndex(filterName) == -1) {
            printf("Unknown filter: %s\n", argv[filterIndex + 1]);
            return 1;
        }
    }
    db->setGlobalInfo(viewName, sortName, filterName);
    View *view = db->getView(viewName);
    view->exportToXML(xmlFile);
    // view is deleted by Database destructor
    delete db;
    return 0;
}

void CommandLine::printUsage()
{
    printf("Usage: portabase [-h | --help | -f file]\n");
    printf("       portabase command [options] fromfile tofile\n");
    printf("  where command is fromxml, toxml, or frommobiledb\n");
    printf("  Valid options for toxml are:\n");
    printf("    -v viewname (apply this view before exporting)\n");
    printf("    -s sortname (apply this sorting before exporting)\n");
    printf("    -f filtername (apply this filter before exporting)\n");
    printf("  Specify -h or --help to receive this message\n");
}
