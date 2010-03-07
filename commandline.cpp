/*
 * commandline.cpp
 *
 * (c) 2003,2008-2009 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file commandline.cpp
 * Source file for CommandLine
 */

#include <QApplication>
#include <QFile>
#include <QObject>
#include <QStringList>
#include "commandline.h"
#include "database.h"
#include "importutils.h"
#include "view.h"
#include "encryption/crypto.h"

/**
 * Constructor.
 */
CommandLine::CommandLine()
{

}

/**
 * Parse the command line arguments and perform the requested action.
 */
int CommandLine::process()
{
    QStringList args = qApp->arguments();
    QStringList importCommands;
    importCommands << "fromxml" << "fromcsv" << "frommobiledb";
    if (importCommands.contains(args[1])) {
        return fromOtherFormat(args);
    }
    else if (args[1] == "toxml" || args[1] == "tocsv") {
        return toOtherFormat(args);
    }
    else if (args[1] == "-h" || args[1] == "--help") {
        printUsage();
        return 0;
    }
    else {
        printUsage();
        return 1;
    }
}

/**
 * Process a command line instruction to import data from another file format.
 *
 * @param args The command line arguments
 * @return The value to return from program execution
 */
int CommandLine::fromOtherFormat(const QStringList &args)
{
    int argc = args.count();
    int numArgs = 4;
    // Check for a password
    int passIndex = args.indexOf("-p");
    if (passIndex != -1) {
        if (argc < passIndex + 4) {
            printUsage();
            return 1;
        }
        numArgs += 2;
    }
    // Check for an encoding specification
    int encIndex = args.indexOf("-e");
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
    QString sourceFile(args[numArgs - 2]);
    QString pbFile(args[numArgs - 1]);
    bool fromcsv = (args[1] == "fromcsv");
    if (fromcsv && !QFile::exists(pbFile)) {
        printf("Named PortaBase file doesn't exist\n");
        return 1;
    }
    Database::OpenResult openResult;
    bool encrypt = passIndex != -1 && !fromcsv;
    Database *db = new Database(pbFile, &openResult, encrypt);
    if (openResult == Database::NewerVersion) {
        if (fromcsv) {
            printf("Unable to open PortaBase file\n");
        }
        else {
            printf("Unable to create/initialize file\n");
        }
        return 1;
    }
    else if (openResult == Database::Encrypted || (passIndex != -1 && !fromcsv)) {
        QString error = db->encryption()->setPassword(args[passIndex + 1], !fromcsv);
        if (!error.isEmpty()) {
            printf("%s", error.toLocal8Bit().data());
            printf("\n");
            return 1;
        }
        error = db->load();
        if (!error.isEmpty()) {
            printf("%s", error.toLocal8Bit().data());
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
    if (args[1] == "fromxml") {
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
    if (!error.isEmpty()) {
        printf("%s", error.toLocal8Bit().data());
        printf("\n");
        if (!data.isEmpty()) {
            printf("%s", (QObject::tr("Problematic row") + ":\n").toLocal8Bit().data());
            printf("%s", data.toLocal8Bit().data());
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

/**
 * Process a command line instruction to export data to another file format.
 *
 * @param args The command line arguments
 * @return The value to return from program execution
 */
int CommandLine::toOtherFormat(const QStringList &args)
{
    int argc = args.count();
    int numArgs = 4;
    // Check for a password
    int passIndex = args.indexOf("-p");
    if (passIndex != -1) {
        if (argc < passIndex + 4) {
            printUsage();
            return 1;
        }
        numArgs += 2;
    }
    // Check for a view
    int viewIndex = args.indexOf("-v");
    if (viewIndex != -1) {
        if (argc < viewIndex + 4) {
            printUsage();
            return 1;
        }
        numArgs += 2;
    }
    // Check for a sorting
    int sortIndex = args.indexOf("-s");
    if (sortIndex != -1) {
        if (argc < sortIndex + 4) {
            printUsage();
            return 1;
        }
        numArgs += 2;
    }
    // Check for a filter
    int filterIndex = args.indexOf("-f");
    if (filterIndex != -1) {
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
    Database::OpenResult openResult;
    Database *db = new Database(pbFile, &openResult);
    if (openResult == Database::NewerVersion) {
        printf("Error opening file: %s\n", args[numArgs - 2].toLocal8Bit().data());
        return 1;
    }
    else if (openResult == Database::Encrypted) {
        if (passIndex == -1) {
            printf("Encrypted file, must provide password\n");
            return 1;
        }
        QString error = db->encryption()->setPassword(args[passIndex + 1], false);
        if (!error.isEmpty()) {
            printf("%s", error.toLocal8Bit().data());
            printf("\n");
            return 1;
        }
        error = db->load();
        if (!error.isEmpty()) {
            printf("%s", error.toLocal8Bit().data());
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
        if (!db->listViews().contains(viewName)) {
            printf("Unknown view: %s\n", viewName.toLocal8Bit().data());
            return 1;
        }
    }
    if (sortIndex != -1) {
        sortName = args[sortIndex + 1];
        if (!db->listSortings().contains(sortName)) {
            printf("Unknown sorting: %s\n", sortName.toLocal8Bit().data());
            return 1;
        }
    }
    if (filterIndex != -1) {
        filterName = args[filterIndex + 1];
        if (!db->listFilters().contains(filterName)) {
            printf("Unknown filter: %s\n", filterName.toLocal8Bit().data());
            return 1;
        }
    }
    db->setGlobalInfo(viewName, sortName, filterName);
    View *view = db->getView(viewName);
    if (args[1] == "toxml") {
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

/**
 * Print usage instructions to the console.
 */
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
