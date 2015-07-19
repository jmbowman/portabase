/*
 * csvutils.h
 *
 * (c) 2002,2008,2013 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file csvutils.h
 * Header file for CSVUtils
 */

#ifndef CSVUTILS_H
#define CSVUTILS_H

#include <QStringList>
#include "calc/calcnode.h"
#include "datatypes.h"

class Database;
class QTextStream;
class View;

/**
 * Utility methods for parsing and generating CSV files.
 */
class CSVUtils
{
public:
    CSVUtils(QChar delimiter=',', const QString &encoding="UTF-8",
             bool headers=false, const QString &lineEnding="\n",
             bool addNewEnumOptions=false);
    ~CSVUtils();

    QStringList parseFile(const QString &filename, Database *db);
    void writeFile(const QString &filename, Database *db, View *view);

private:
    void initialize(Database *db, const QString &filename);
    bool addRow(Database *db);
    void applyCodec(QTextStream *stream);
    QString encodeRow(QStringList row);
    QString encodeCell(QString content);

private:
    QChar m_textquote; /**< The character used to quote fields */
    bool m_add_options; /**< True if new enum options should be automatically added */
    QChar m_delimiter; /**< The character used to separate fields */
    QString m_encoding; /**< The text encoding to use when reading or writing a file */
    bool m_headers; /**< True if the CSV file has (or should have) a row of column headers */
    QString m_line_ending; /**< The character(s) used to designate the end of a record */
    QStringList colNames; /**< List of database column names, cached for convenience */
    int colCount; /**< Number of columns in the database, cached for convenience */
    int endStringCount; /**< Number of string or note fields at the end of each record */
    bool headersParsed; /**< True if the header row has already been parsed */
    int rowNum; /**< Number of the row currently being parsed; used in error messages */
    QString message; /**< Error message, if any, from loading the last parsed record */
    QString rowString; /**< The record currently being parsed; used in error messages */
    QStringList row; /**< The fields of the record currently being parsed */
    IntList addedIds;  /**< IDs of all rows added while parsing a file */
    IntList types; /**< List of database column types, cached for convenience */
    CalcNodeList calcs; /**< List of calculated column definition tree root nodes */
    IntList calcDecimals; /**< List of decimal places to show for each calculated column */
    int calcCount; /**< Number of calculated columns */
};

#endif
