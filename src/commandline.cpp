/*
 * commandline.cpp
 *
 * (c) 2003,2008-2011,2013,2015 by Jeremy Bowman <jmbowman@alum.mit.edu>
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
#include <QFileInfo>
#include <QObject>
#include <QStringList>
#include <QTextCodec>
#include "commandline.h"
#include "csvutils.h"
#include "database.h"
#include "importutils.h"
#include "view.h"
#include "encryption/crypto.h"

/**
 * Constructor.
 */
CommandLine::CommandLine() : argc(0), numArgs(4)
{

}

/**
 * Parse the command line arguments and perform the requested action.
 * @return The program execution return value
 */
int CommandLine::process()
{
    QStringList args = qApp->arguments();
    QStringList importCommands;
    QStringList exportCommands;
    importCommands << "fromxml" << "fromcsv" << "frommobiledb";
    exportCommands << "toxml" << "tocsv" << "tohtml";
    if (importCommands.contains(args[1])) {
        return fromOtherFormat(args);
    }
    else if (exportCommands.contains(args[1])) {
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
 * Look for the specified command-line option and return its index among the
 * arguments.
 *
 * @param option The option string to look for
 * @param takesArgument True if this option requires a parameter to be provided
 * @return The index at which the option was found, -1 if not found, or -2 if
 *         a required argument is missing
 */
int CommandLine::parseOption(const QString &option, bool takesArgument)
{
    int index = qApp->arguments().indexOf(option);
    if (index == -1) {
        return -1;
    }
    if (takesArgument && (argc < index + 4)) {
        return -2;
    }
    numArgs += (takesArgument ? 2 : 1);
    return index;
}

/**
 * Verify that the provided text encoding (if any) is one that is understood
 * by PortaBase.
 *
 * @param argIndex The position index of the encoding parameter
 * @return The encoding if valid, "UTF-8" if none was specified, or a null
 *         string if the provided encoding is not recognized
 */
QString CommandLine::validateEncoding(int argIndex)
{
    if (argIndex == -1) {
        return QString("UTF-8");
    }
    QString encoding = qApp->arguments()[argIndex];
    if (!QTextCodec::codecForName(encoding.toLatin1())) {
        printf("Unknown text encoding: %s\n", encoding.toLocal8Bit().data());
        return QString();
    }
    return encoding;
}

/**
 * Process a command line instruction to import data from another file format.
 *
 * @param args The command line arguments
 * @return The value to return from program execution
 */
int CommandLine::fromOtherFormat(const QStringList &args)
{
    argc = args.count();
    int passIndex = parseOption("-p");
    int encIndex = parseOption("-e");
    int delIndex = parseOption("-d");
    int enumsIndex = parseOption("--add-unknown-enum-options", false);
    int headersIndex = parseOption("--headers", false);
    if (passIndex == -2 || encIndex == -2 || delIndex == -2
        || enumsIndex == -2 || headersIndex == -2 || argc != numArgs) {
        printUsage();
        return 1;
    }
    QString encoding = validateEncoding(encIndex);
    if (encoding.isNull()) {
        return 1;
    }
    QString sourceFile(args[numArgs - 2]);
    QString pbFile(args[numArgs - 1]);
    bool fromcsv = (args[1] == "fromcsv");
    if (fromcsv) {
        if (!QFile::exists(pbFile)) {
            printf("Named PortaBase file doesn't exist\n");
            return 1;
        }
        QFileInfo info(pbFile);
        if (!info.isWritable()) {
            printf("Named PortaBase file can't be written to");
            return 1;
        }
    }
    Database::OpenResult openResult;
    bool encrypt = passIndex != -1 && !fromcsv;
    Database *db = new Database(pbFile, &openResult, encrypt);
    if (openResult == Database::NewerVersion || openResult == Database::Failure) {
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
    if (args[1] == "fromxml") {
        error = ImportUtils::importXML(sourceFile, db);
    }
    else if (fromcsv) {
        QChar delimiter = ',';
        if (delIndex != -1) {
            QString delimArg = args[delIndex + 1];
            if (delimArg == "tab") {
                delimiter = '\t';
            }
            else {
                delimiter = delimArg[0];
            }
        }
        CSVUtils csv(delimiter, encoding, headersIndex != -1, "\n",
                     enumsIndex != -1);
        QStringList result = db->importFromCSV(sourceFile, &csv);
        int count = result.count();
        if (count > 0) {
            error = result[0];
        }
        if (count > 1) {
            data = result[1];
        }
    }
    else {
        error = ImportUtils::importMobileDB(sourceFile, db);
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
    argc = args.count();
    int passIndex = parseOption("-p");
    int viewIndex = parseOption("-v");
    int sortIndex = parseOption("-s");
    int filterIndex = parseOption("-f");
    int encIndex = parseOption("-e");
    int delIndex = parseOption("-d");
    int endIndex = parseOption("-l");
    int headersIndex = parseOption("--headers", false);
    if (passIndex == -2 || viewIndex == -2 || sortIndex == -2
        || filterIndex == -2 || encIndex == -2 || delIndex == -2
        || endIndex == -2 || headersIndex == -2 || argc != numArgs) {
        printUsage();
        return 1;
    }
    QString encoding = validateEncoding(encIndex);
    if (encoding.isNull()) {
        return 1;
    }
    QString pbFile(args[numArgs - 2]);
    QString outputFile(args[numArgs - 1]);
    Database::OpenResult openResult;
    Database *db = new Database(pbFile, &openResult);
    if (openResult == Database::NewerVersion || openResult == Database::Failure) {
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
    QString argument;
    if (args[1] == "toxml") {
        view->exportToXML(outputFile);
    }
    else if (args[1] == "tohtml") {
        view->exportToHTML(outputFile);
    }
    else {
        QChar delimiter = ',';
        if (delIndex != -1) {
            argument = args[delIndex + 1];
            if (argument == "tab") {
                delimiter = '\t';
            }
            else {
                delimiter = argument[0];
            }
        }
        QString lineEnding("\n");
        if (endIndex != -1) {
            argument = args[endIndex + 1];
            if (argument == "crlf") {
                lineEnding = "\r\n";
            }
        }
        view->prepareData();
        CSVUtils csv(delimiter, encoding, headersIndex != -1, lineEnding);
        csv.writeFile(outputFile, db, view);
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
    printf("Usage: portabase [-h | --help | file]\n");
    printf("       portabase command [-p password] [options] fromfile tofile\n");
    printf("  where command is fromxml, toxml, fromcsv, tocsv, tohtml, or frommobiledb\n");
    printf("  Valid options for toxml, tocsv, and tohtml are:\n");
    printf("    -v viewname (apply this view before exporting)\n");
    printf("    -s sortname (apply this sorting before exporting)\n");
    printf("    -f filtername (apply this filter before exporting)\n");
    printf("  Options for fromcsv and tocsv:\n");
    printf("    --headers (if there is or should be a row of column headers)\n");
    printf("    -d delimiter (typically 'tab' if not using the default ',')\n");
    printf("    -e encoding (UTF-8 (the default), Latin-1, Shift-JIS, etc.)\n");
    printf("  Additional option for fromcsv:\n");
    printf("    --add-unknown-enum-options (automatically add new enum values encountered)\n");
    printf("  Additional option for tocsv:\n");
    printf("    -l line_ending ('crlf' for Windows-style, default is UNIX/Mac-style)\n");
    printf("  When using fromcsv, \"tofile\" must be an existing PortaBase file.\n");
    printf("  Specify -h or --help to receive this message\n");
}
