/*
 * xmlexport.cpp
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qfile.h>
#include <qtextstream.h>
#include "database.h"
#include "datatypes.h"
#include "xmlexport.h"

XMLExport::XMLExport(Database *source, QString filename, QStringList currentCols) : db(source), indentUnit(" ")
{
    cols = currentCols;
    fieldElements.append("s");
    fieldElements.append("i");
    fieldElements.append("f");
    fieldElements.append("b");
    fieldElements.append("n");
    fieldElements.append("d");
    fieldElements.append("t");
    file = new QFile(filename);
    file->open(IO_WriteOnly);
    output = new QTextStream(file);
    output->setEncoding(QTextStream::UnicodeUTF8);
    *output << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    *output << "<portabase>\n";
    indent = indentUnit;
}

XMLExport::~XMLExport()
{
    *output << "</portabase>\n";
    file->close();
    delete output;
    delete file;
}

void XMLExport::addGlobalView(c4_View &global)
{
    *output << (indent + "<global>\n");
    indent += indentUnit;
    int count = global.NumProperties();
    for (int i = 0; i < count; i++) {
        const c4_Property prop = global.NthProperty(i);
        QString name = getPropName(prop);
        if (name == "gcrypt") {
            continue;
        }
        *output << (indent + "<" + name + ">");
        *output << getValue(global, "_" + name, prop.Type(), 0);
        *output << ("</" + name + ">\n");
    }
    indent = indent.remove(0, indentUnit.length());
    *output << (indent + "</global>\n");
}

void XMLExport::addView(QString name, c4_View view)
{
    *output << (indent + "<" + name + ">\n");
    indent += indentUnit;
    // strip off trailing 's'
    QString rowElement = name.left(name.length() - 1);
    int colCount = view.NumProperties();
    QStringList elementNames;
    QStringList propNames;
    char *types = new char[colCount];
    int i;
    for (i = 0; i < colCount; i++) {
        const c4_Property prop = view.NthProperty(i);
        QString element = getPropName(prop);
        elementNames.append(element);
        propNames.append("_" + element);
        types[i] = prop.Type();
    }
    int rowCount = view.GetSize();
    for (i = 0; i < rowCount; i++) {
        *output << (indent + "<" + rowElement + ">\n");
        indent += indentUnit;
        for (int j = 0; j < colCount; j++) {
            *output << (indent + "<" + elementNames[j] + ">");
            *output << getValue(view, propNames[j], types[j], i);
            *output << ("</" + elementNames[j] + ">\n");
        }
        indent = indent.remove(0, indentUnit.length());
        *output << (indent + "</" + rowElement + ">\n");
    }
    indent = indent.remove(0, indentUnit.length());
    *output << (indent + "</" + name + ">\n");
}

void XMLExport::addDataView(c4_View &fullView, c4_View &filteredView,
                            int *colIds, int *colTypes, QStringList idStrings)
{
    *output << (indent + "<data>\n");
    indent += indentUnit;
    int rowCount = fullView.GetSize();
    int colCount = idStrings.count();
    int viewColCount = cols.count();
    char *typeChars = new char[colCount];
    int i;
    for (i = 0; i < colCount; i++) {
        int type = colTypes[i];
        if (type == INTEGER || type == BOOLEAN || type == DATE
                || type == TIME) {
            typeChars[i] = 'I';
        }
        else {
            typeChars[i] = 'S';
        }
    }
    c4_IntProp Id("_id");
    for (i = 0; i < rowCount; i++) {
        *output << (indent + "<r");
        int rowId = Id (fullView[i]);
        if (filteredView.Find(Id [rowId]) == -1) {
            *output << " h=\"y\"";
        }
        *output << ">\n";
        indent += indentUnit;
        for (int j = 0; j < colCount; j++) {
            int type = colTypes[j];
            QString element("e");
            if (type < FIRST_ENUM) {
                element = fieldElements[type];
            }
            QString value = getValue(fullView, idStrings[j], typeChars[j], i);
            *output << (indent + "<" + element + " c=\"");
            *output << QString::number(colIds[j]);
            if (element == "d") {
                QString stringRep = db->dateToString(value.toInt());
                *output << ("\" s=\"" + stringRep);
            }
            if (element == "t") {
                QString stringRep = db->timeToString(value.toInt());
                *output << ("\" s=\"" + stringRep);
            }
            if (j >= viewColCount) {
                *output << "\" h=\"y";
            }
            *output << ("\">" + value + "</" + element + ">\n");
        }
        indent = indent.remove(0, indentUnit.length());
        *output << (indent + "</r>\n");
    }
    indent = indent.remove(0, indentUnit.length());
    *output << (indent + "</data>\n");
    delete[] colIds;
    delete[] colTypes;
    delete[] typeChars;
}

QString XMLExport::getPropName(const c4_Property prop)
{
    QString name = QString::fromUtf8(prop.Name());
    // strip leading '_'
    return name.right(name.length() - 1);
}

QString XMLExport::getValue(c4_View &view, QString name, char type, int row)
{
    if (type == 'I') {
        c4_IntProp prop(name.utf8());
        int value = prop (view[row]);
        return QString::number(value);
    }
    else {
        c4_StringProp prop(name.utf8());
        QString value = QString::fromUtf8(prop (view[row]));
        int length = value.length();
        QString result = "";
        for (int i = 0; i < length; i++) {
            const QChar c = value[i];
            if (c == '<') {
               result += "&lt;";
            }
            else if (c == '>') {
                result += "&gt;";
            }
            else if (c == '&') {
                result += "&amp;";
            }
            else {
                result += c;
            }
        }
        return result;
    }
}
