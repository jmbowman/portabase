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

#include <qfile.h>
#include <qobject.h>
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
    else if (argv[1] == QCString("fromcsv")) {
        return fromOtherFormat(argc, argv);
    }
    else if (argv[1] == QCString("frommobiledb")) {
        return fromOtherFormat(argc, argv);
    }
    else if (argv[1] == QCString("toxml")) {
        return toOtherFormat(argc, argv);
    }
    else if (argv[1] == QCString("tocsv")) {
        return toOtherFormat(argc, argv);
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
    QStringList args;
    for (int i = 0; i < argc; i++) {
        args.append(argv[i]);
    }
    int numArgs = 4;
    int passIndex = args.findIndex("-p");
    if (passIndex != -1) {
        if (argc < passIndex + 4) {
            printUsage();
            return 1;
        }
        numArgs += 2;
    }
    int encIndex = args.findIndex("-e");
    if (encIndex != -1) {
        if (argc < encIndex + 4) {
            printUsage();
            return 1;
        }
        numArgs += 2;
    }
    if (argc != numArgs) {
        printUsage();
        return 1;
    }
    QString sourceFile(argv[numArgs - 2]);
    QString pbFile(argv[numArgs - 1]);
    bool fromcsv = (argv[1] == QCString("fromcsv"));
    if (fromcsv && !QFile::exists(pbFile)) {
        printf("Named PortaBase file doesn't exist\n");
        return 1;
    }
    int openResult;
    int encrypt = (passIndex != -1 && !fromcsv) ? 1 : 0;
    Database *db = new Database(pbFile, &openResult, encrypt);
    if (openResult == OPEN_NEWER_VERSION) {
        if (fromcsv) {
            printf("Unable to open PortaBase file\n");
        }
        else {
            printf("Unable to create/initialize file\n");
        }
        return 1;
    }
    else if (openResult == OPEN_ENCRYPTED || (passIndex != -1 && !fromcsv)) {
        QString error = db->setPassword(args[passIndex + 1], !fromcsv);
        if (error != "") {
            printf(error.local8Bit());
            printf("\n");
            return 1;
        }
        error = db->load();
        if (error != "") {
            printf(error.local8Bit());
            printf("\n");
            return 1;
        }
    }
    else {
        db->load();
    }
    QString error = "";
    QString data = "";
    ImportUtils utils;
    if (argv[1] == QCString("fromxml")) {
        error = utils.importXML(sourceFile, db);
    }
    else if (fromcsv) {
        QString encoding = "UTF-8";
        if (encIndex != -1) {
            encoding = args[encIndex + 1];
        }
        QStringList result = db->importFromCSV(sourceFile, encoding);
        int count = result.count();
        if (count > 0) {
            error = result[0];
        }
        if (count > 1) {
            data = result[1];
        }
    }
    else {
        error = utils.importMobileDB(sourceFile, db);
    }
    if (error != "") {
        printf(error.local8Bit());
        printf("\n");
        if (data != "") {
            printf((QObject::tr("Problematic row") + ":\n").local8Bit());
            printf(data.local8Bit());
            printf("\n");
        }
        if (!fromcsv) {
            QFile::remove(pbFile);
        }
        delete db;
        return 1;
    }
    db->commit();
    delete db;
    return 0;
}

int CommandLine::toOtherFormat(int argc, char **argv)
{
    QStringList args;
    for (int i = 0; i < argc; i++) {
        args.append(argv[i]);
    }
    int numArgs = 4;
    int passIndex = args.findIndex("-p");
    if (passIndex != -1) {
        if (argc < passIndex + 4) {
            printUsage();
            return 1;
        }
        numArgs += 2;
    }
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
    QString outputFile(args[numArgs - 1]);
    int openResult;
    Database *db = new Database(pbFile, &openResult);
    if (openResult == OPEN_NEWER_VERSION) {
        printf("Error opening file: %s\n", argv[numArgs - 2]);
        return 1;
    }
    else if (openResult == OPEN_ENCRYPTED) {
        if (passIndex == -1) {
            printf("Encrypted file, must provide password\n");
            return 1;
        }
        QString error = db->setPassword(args[passIndex + 1], FALSE);
        if (error != "") {
            printf(error.local8Bit());
            printf("\n");
            return 1;
        }
        error = db->load();
        if (error != "") {
            printf(error.local8Bit());
            printf("\n");
            return 1;
        }
    }
    else {
        db->load();
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
    if (argv[1] == QCString("toxml")) {
        view->exportToXML(outputFile);
    }
    else {
        view->prepareData();
        view->exportToCSV(outputFile);
    }
    // view is deleted by Database destructor
    delete db;
    return 0;
}

void CommandLine::printUsage()
{
    printf("Usage: portabase [-h | --help | -f file]\n");
    printf("       portabase command [-p password] [options] fromfile tofile\n");
    printf("  where command is fromxml, toxml, fromcsv, tocsv, or frommobiledb\n");
    printf("  Valid options for toxml and tocsv are:\n");
    printf("    -v viewname (apply this view before exporting)\n");
    printf("    -s sortname (apply this sorting before exporting)\n");
    printf("    -f filtername (apply this filter before exporting)\n");
    printf("  There is one option for fromcsv:\n");
    printf("    -e encoding (UTF-8 or Latin-1; default is UTF-8)\n");
    printf("  When using fromcsv, \"tofile\" must be an existing PortaBase file.\n");
    printf("  Specify -h or --help to receive this message\n");
}
