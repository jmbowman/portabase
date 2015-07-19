/*
 * xmlexport.cpp
 *
 * (c) 2003,2009-2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file xmlexport.cpp
 * Source file for XMLExport
 */

#include <QFile>
#include <QTextDocument>
#include <QTextStream>
#include "database.h"
#include "datatypes.h"
#include "formatting.h"
#include "xmlexport.h"

/**
 * Constructor.
 *
 * @param source The database to export from
 * @param filename The XML file to export to
 * @param currentCols Ordered list of column names in the current view
 */
XMLExport::XMLExport(Database *source, const QString &filename,
  const QStringList &currentCols) : db(source), indentUnit(" ")
{
    cols = currentCols;
    fieldElements.append("s");
    fieldElements.append("i");
    fieldElements.append("f");
    fieldElements.append("b");
    fieldElements.append("n");
    fieldElements.append("d");
    fieldElements.append("t");
    fieldElements.append("c");
    fieldElements.append("q");
    fieldElements.append("p");
    file = new QFile(filename);
    file->open(QFile::WriteOnly);
    output = new QTextStream(file);
    output->setCodec("UTF-8");
    *output << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    *output << "<portabase>\n";
    indent = indentUnit;
    utils.setExportPaths(filename);
}

/**
 * Destructor.
 */
XMLExport::~XMLExport()
{
    *output << "</portabase>\n";
    file->close();
    delete output;
    delete file;
}

/**
 * Add a &lt;global&gt; element representing the "_global" Metakit view.
 *
 * @param global The Metakit view to export
 */
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

/**
 * Add an element representing the provided Metakit view.
 *
 * @param name The name to use for the generated element
 * @param view The Metakit view to export
 */
void XMLExport::addView(const QString &name, c4_View view)
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

/**
 * Add an element representing the actual user data table.
 *
 * @param fullView The Metakit view to export
 * @param filteredView The Metakit view resulting from applying the current
 *                     filter
 * @param colIds Ordered array of field ID numbers (array deleted here)
 * @param colTypes Ordered array of field type IDs (array deleted here)
 * @param idStrings Ordered list of Metakit data field identifiers
 * @param colNames Ordered list of field names in the database
 */
void XMLExport::addDataView(c4_View &fullView, c4_View &filteredView,
                            int *colIds, int *colTypes,
                            const QStringList &idStrings,
                            const QStringList &colNames)
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
                || type == TIME || type == SEQUENCE) {
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
                QString stringRep = Formatting::dateToString(value.toInt());
                *output << ("\" s=\"" + stringRep);
            }
            else if (element == "t") {
                QString stringRep = Formatting::timeToString(value.toInt());
                *output << ("\" s=\"" + stringRep);
            }
            else if (element == "p") {
                value = utils.exportImage(db, rowId, colNames[j], value);
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

/**
 * Get the name to use for the export element corresponding to the given
 * Metakit property.  Generally, this only involves stripping the leading
 * underscore from the property name.
 *
 * @param prop The Metakit property being exported
 * @return The name to use for the corresponding exported element
 */
QString XMLExport::getPropName(const c4_Property prop)
{
    QString name = QString::fromUtf8(prop.Name());
    // strip leading '_'
    return name.right(name.length() - 1);
}

/**
 * Get a value from a Metakit view cell for export.  Escapes special XML
 * characters in the value where appropriate.
 *
 * @param view The view being exported
 * @param name The name of the Metakit property being exported from
 * @param type The Metakit property type character for this property
 * @param row The position index in the view of the row being exported
 * @return The specified data value, ready for insertion into an XML file
 */
QString XMLExport::getValue(c4_View &view, const QString &name, char type, int row)
{
    if (type == 'I') {
        c4_IntProp prop(name.toUtf8());
        int value = prop (view[row]);
        return QString::number(value);
    }
    else {
        c4_StringProp prop(name.toUtf8());
        QString value = QString::fromUtf8(prop (view[row]));
        return Qt::escape(value);
    }
}
