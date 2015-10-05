/*
 * xmlimport.h
 *
 * (c) 2003,2009 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file xmlimport.h
 * Header file for XMLImport
 */

#ifndef XMLIMPORT_H
#define XMLIMPORT_H

#include <QMap>
#include <QObject>
#include <QStringList>
#include <QXmlAttributes>
#include <QXmlDefaultHandler>
#include <QXmlLocator>
#include <QXmlParseException>
#include "datatypes.h"

class CalcNode;
class Database;
class Filter;

typedef QMap<QString,QMap<int,QString> > IndexMap;
typedef QMap<QString,Filter*> FilterMap;

/**
 * Handler for parsing %PortaBase XML data files.  XML is used as an
 * import/export format for %PortaBase, allowing the content of a database
 * to be represented in a plain text format for manual or automated
 * manipulation.  This class assumes that data is being loaded into an
 * essentially empty %PortaBase file; to add data to an existing file, use
 * CSV instead.
 */
class XMLImport : public QObject, public QXmlDefaultHandler
{
    Q_OBJECT
public:
    XMLImport(Database *dbase);

    bool startDocument();
    bool startElement(const QString &namespaceURI, const QString &localName,
                      const QString &qName, const QXmlAttributes &atts);
    bool endElement(const QString &namespaceURI, const QString &localName,
                    const QString &qName);
    bool characters (const QString &ch);
    QString errorString();
    QString formattedError();
    bool fatalError(const QXmlParseException &exception);
    void setDocumentLocator(QXmlLocator *locator);

private:
    void buildParentsMap();
    bool updateGlobalRecord();
    bool addEnum();
    bool addEnumOption();
    bool addColumn();
    bool addView();
    bool addViewColumn();
    bool addSort();
    bool addSortColumn();
    bool addFilter();
    bool addFilterCondition();
    bool addCalc();
    bool addCalcNode();
    bool addRow();
    bool validateName(const QString &name);
    bool validateGlobal();
    bool validateColumns();
    bool validateEnums();
    bool validateViewColumns();
    bool validateSortColumns();
    bool validateFilterConditions();
    bool validateCalcNodes();
    QString missingSection(const QString &before, int afterIndex);
    bool isValidDefault(const QString &cname, int ctype,
                        const QString &cdefault);
    bool isValidOperator(int type, int op, int cs);
    QString isValidCalcNode(CalcNode *node);
    bool containsDuplicate(const QStringList &names, const QString &element);
    bool containsDuplicate(const IntList &names, const QString &element);
    bool validateIndexMap(const QString &intElement,
                          const QString &stringElement);
    bool setField(const QString &name);
    QString getField(const QString &name);
    QString getOptionalField(const QString &name, const QString &defaultVal);
    QString getDataField(int columnId);

private:
    Database *db; /**< The database to load data into */
    QXmlLocator *xmlLocator; /**< Current location in the file being parsed */
    QString error; /**< Error message, empty if no errors encountered yet */
    QString text; /**< The text content of the element being parsed */
    QStringList ancestors; /**< Element names from the root to here */
    QString parent; /**< The name of the current element's parent element */
    QString lastSection; /**< Name of the last top-level element parsed */
    QMap<QString,QString> fields; /**< Name/ID to value map for the current element */
    int gversion; /**< Version of the PortaBase file format this data was intended for */
    QString gview; /**< Name of the database view to set as current */
    QString gsort; /**< Name of the sorting to set as current */
    QString gfilter; /**< Name of the filter to set as current */
    QMap<int,int> enumMap; /**< Map of enum indexes to IDs */
    IntList enumIds; /**< List of enum IDs */
    QMap<int,int> columnMap; /**< Map of column indexes to IDs */
    QStringList colNames; /**< List of column names */
    IntList idList; /**< List of column IDs */
    int colCount; /**< Number of columns in the database */
    IndexMap indexMap; /**< Data structure used while parsing columns, enums, etc. */
    FilterMap filterMap; /**< Filter name to filter object mapping */
    QString colId; /**< Column ID of the data field being parsed */
    QMap<QString,QString> parents; /**< Allowed child-to-parent element name mappings */
    QStringList sections; /**< List of top-level element names */
    QStringList optionalSections; /**< Names of section elements which may be omitted */
    QStringList containers; /**< Names of elements whose child elements contain text data */
    QStringList dataFields; /**< Names of elements representing data fields */
    QStringList booleans; /**< Names of elements containing boolean value text */
    QStringList integers; /**< Names of elements containing integer value text */
    QStringList nonNegativeIntegers; /**< Names of elements containing non-negative integer value text */
    QStringList positiveIntegers; /**< Names of elements containing positive integer value text */
};

#endif
