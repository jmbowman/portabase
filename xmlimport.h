/*
 * xmlimport.h
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef XMLIMPORT_H
#define XMLIMPORT_H

#include <qmap.h>
#include <qstringlist.h>
#include <qxml.h>
#include "datatypes.h"

class Database;
class Filter;

typedef QMap<QString,QMap<int,QString> > IndexMap;
typedef QMap<QString,Filter*> FilterMap;

class XMLImport : public QXmlDefaultHandler
{
public:
    XMLImport(Database *dbase);
    ~XMLImport();

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
    bool addRow();
    bool validateName(const QString &name);
    bool validateGlobal();
    bool validateColumns();
    bool validateEnums();
    bool validateViewColumns();
    bool validateSortColumns();
    bool validateFilterConditions();
    bool isValidDefault(const QString &cname, int ctype,
                        const QString &cdefault);
    bool isValidOperator(int type, int op, int cs);
    bool containsDuplicate(const QStringList &names, const QString &element);
    bool containsDuplicate(const IntList &names, const QString &element);
    bool validateIndexMap(const QString &intElement,
                          const QString &stringElement);
    bool setField(const QString &name);
    QString getField(const QString &name);
    QString getOptionalField(const QString &name, const QString &defaultVal);
    QString getDataField(int columnId);

private:
    Database *db;
    QXmlLocator *xmlLocator;
    QString error;
    QString text;
    QStringList ancestors;
    QString parent;
    QString lastSection;
    QMap<QString,QString> fields;
    int gversion;
    QString gview;
    QString gsort;
    QString gfilter;
    QMap<int,int> enumMap;
    IntList enumIds;
    QMap<int,int> columnMap;
    QStringList colNames;
    IntList idList;
    int colCount;
    IndexMap indexMap;
    FilterMap filterMap;
    QString colId;
    QMap<QString,QString> parents;
    QStringList sections;
    QStringList containers;
    QStringList dataFields;
    QStringList booleans;
    QStringList integers;
    QStringList nonNegativeIntegers;
    QStringList positiveIntegers;
};

#endif
