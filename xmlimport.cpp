/*
 * xmlimport.cpp
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qobject.h>
#include "condition.h"
#include "database.h"
#include "filter.h"
#include "xmlimport.h"

XMLImport::XMLImport(Database *dbase) : QXmlDefaultHandler(), db(dbase), error("")
{
    buildParentsMap();

    sections.append("global");
    sections.append("enums");
    sections.append("enumoptions");
    sections.append("columns");
    sections.append("views");
    sections.append("viewcolumns");
    sections.append("sorts");
    sections.append("sortcolumns");
    sections.append("filters");
    sections.append("filterconditions");
    sections.append("data");

    containers.append("global");
    containers.append("enum");
    containers.append("enumoption");
    containers.append("column");
    containers.append("view");
    containers.append("viewcolumn");
    containers.append("sort");
    containers.append("sortcolumn");
    containers.append("filter");
    containers.append("filtercondition");
    containers.append("r");

    dataFields.append("s");
    dataFields.append("i");
    dataFields.append("f");
    dataFields.append("b");
    dataFields.append("n");
    dataFields.append("d");
    dataFields.append("t");
    dataFields.append("e");

    booleans.append("scdesc");
    booleans.append("fccase");
    booleans.append("b");

    integers.append("i");
    integers.append("d");
    integers.append("t");

    nonNegativeIntegers.append("eindex");
    nonNegativeIntegers.append("eoindex");
    nonNegativeIntegers.append("cindex");
    nonNegativeIntegers.append("ctype");
    nonNegativeIntegers.append("cid");
    nonNegativeIntegers.append("vcindex");
    nonNegativeIntegers.append("scindex");
    nonNegativeIntegers.append("fcposition");
    nonNegativeIntegers.append("fcoperator");

    positiveIntegers.append("gversion");
    positiveIntegers.append("eid");
    positiveIntegers.append("vrpp");
    positiveIntegers.append("vdeskrpp");
    positiveIntegers.append("vcwidth");
    positiveIntegers.append("vcdeskwidth");
}

XMLImport::~XMLImport()
{

}

QString XMLImport::errorString()
{
    return error;
}

QString XMLImport::formattedError()
{
    if (error != "") {
        QString fullError = QObject::tr("Error at") + ": ";
        fullError += QObject::tr("Line") + " ";
        fullError += QString::number(xmlLocator->lineNumber()) + ", ";
        fullError += QObject::tr("Column") + " ";
        fullError += QString::number(xmlLocator->columnNumber()) + "\n";
        fullError += error;
        return fullError;
    }
    return error;
}

bool XMLImport::startDocument()
{
    lastSection = "";
    error = "";
    gview = "";
    gsort = "";
    gfilter = "";
    ancestors.clear();
    fields.clear();
    enumMap.clear();
    enumIds.clear();
    columnMap.clear();
    idList.clear();
    filterMap.clear();
    return TRUE;
}

bool XMLImport::startElement(const QString&, const QString&,
                             const QString &qName, const QXmlAttributes &atts)
{
    int count = ancestors.count();
    if (qName == "portabase") {
        if (count == 0) {
            ancestors.append(qName);
            return TRUE;
        }
        else {
            error = "portabase " + QObject::tr("only allowed as root element");
            return FALSE;
        }
    }
    if (!parents.contains(qName)) {
        error = QObject::tr("Unknown element") + ": " + qName;
        return FALSE;
    }
    parent = ancestors[count - 1];
    if (parents[qName] != parent) {
        error = qName + " " + QObject::tr("not allowed as child of") + " "
                + parent;
        return FALSE;
    }
    int sectionIndex = sections.findIndex(qName);
    if (sectionIndex != -1) {
        if (sectionIndex > 0) {
            QString previous = sections[sectionIndex - 1];
            if (lastSection != previous) {
                error = QObject::tr("Missing element") + ": " + previous;
                return FALSE;
            }
        }
        lastSection = qName;
    }
    ancestors.append(qName);
    if (dataFields.findIndex(qName) != -1) {
        bool ok = FALSE;
        colId = atts.value("c");
        colId.toInt(&ok);
        if (!ok) {
            error = QObject::tr("\"c\" attribute is missing or non-integer");
            return FALSE;
        }
    }
    if (containers.findIndex(qName) != -1) {
        fields.clear();
    }
    if (qName == "enums" || qName == "views" || qName == "sorts"
            || qName == "filters") {
        indexMap.clear();
    }
    text = "";
    return TRUE;
}

bool XMLImport::endElement(const QString&, const QString&,
                           const QString &qName)
{
    ancestors.remove(qName);
    if (!setField(qName)) {
        return FALSE;
    }
    if (qName == "global") {
        return updateGlobalRecord();
    }
    else if (qName == "enum") {
        return addEnum();
    }
    else if (qName == "enums") {
        return validateEnums();
    }
    else if (qName == "enumoption") {
        return addEnumOption();
    }
    else if (qName == "enumoptions") {
        return validateIndexMap("eoindex", "eotext");
    }
    else if (qName == "column") {
        return addColumn();
    }
    else if (qName == "columns") {
        return validateColumns();
    }
    else if (qName == "view") {
        return addView();
    }
    else if (qName == "viewcolumn") {
        return addViewColumn();
    }
    else if (qName == "viewcolumns") {
        return validateViewColumns();
    }
    else if (qName == "sort") {
        return addSort();
    }
    else if (qName == "sortcolumn") {
        return addSortColumn();
    }
    else if (qName == "sortcolumns") {
        return validateSortColumns();
    }
    else if (qName == "filter") {
        return addFilter();
    }
    else if (qName == "filtercondition") {
        return addFilterCondition();
    }
    else if (qName == "filterconditions") {
        return validateFilterConditions();
    }
    else if (qName == "r") {
        return addRow();
    }
    else if (dataFields.findIndex(qName) != -1) {
        fields.insert(colId, text);
        return TRUE;
    }
    else {
        fields.insert(qName, text);
        return TRUE;
    }
}

bool XMLImport::characters(const QString &ch)
{
    text += ch;
    return TRUE;
}

bool XMLImport::fatalError(const QXmlParseException &exception)
{
    if (error == "") {
        error = exception.message();
    }
    return FALSE;
}

void XMLImport::setDocumentLocator(QXmlLocator *locator)
{
    xmlLocator = locator;
}

bool XMLImport::updateGlobalRecord()
{
    gversion = getField("gversion").toInt();
    gview = getField("gview");
    gsort = getField("gsort");
    gfilter = getField("gfilter");
    if (error != "") {
        return FALSE;
    }
    if (gversion < 7 || gversion > FILE_VERSION) {
        error = QObject::tr("Unsupported") + " gversion: "
                + QString::number(gversion);
        return FALSE;
    }
    db->setGlobalInfo(gview, gsort, gfilter);
    return TRUE;
}

bool XMLImport::addEnum()
{
    int eindex = getField("eindex").toInt();
    QString ename = getField("ename");
    int eid = getField("eid").toInt();
    if (error != "") {
        return FALSE;
    }
    if (!validateName(ename)) {
        return FALSE;
    }
    if (eid < FIRST_ENUM) {
        error = QObject::tr("Invalid") + " eid: " + QString::number(eid);
        return FALSE;
    }
    if (indexMap.contains(ename)) {
        error = QObject::tr("Duplicate") + " ename: " + ename;
        return FALSE;
    }
    QMap<int,QString> mapping;
    indexMap.insert(ename, mapping);
    // empty option list; enum options will be added later
    QStringList options;
    db->addEnum(ename, options, eindex, eid);
    enumMap.insert(eindex, eid);
    return TRUE;
}

bool XMLImport::addEnumOption()
{
    QString idString = getField("eoenum");
    int eoenum = idString.toInt();
    int eoindex = getField("eoindex").toInt();
    QString eotext = getField("eotext");
    if (error != "") {
        return FALSE;
    }
    if (!validateName(eotext)) {
        return FALSE;
    }
    if (enumIds.findIndex(eoenum) == -1) {
        error = QObject::tr("Invalid") + " eoenum: " + idString;
        return FALSE;
    }
    indexMap[idString].insert(eoindex, eotext);
    db->addEnumOption(db->getEnumName(eoenum), eotext, eoindex);
    return TRUE;
}

bool XMLImport::addColumn()
{
    int cindex = getField("cindex").toInt();
    QString cname = getField("cname");
    int ctype = getField("ctype").toInt();
    QString cdefault = getField("cdefault");
    int cid = getField("cid").toInt();
    if (error != "") {
        return FALSE;
    }
    if (!validateName(cname)) {
        return FALSE;
    }
    if (ctype > TIME) {
        if (enumIds.findIndex(ctype) == -1) {
            error = QObject::tr("Invalid") + " ctype: "
                    + QString::number(ctype);
            return FALSE;
        }
    }
    if (!isValidDefault(cname, ctype, cdefault)) {
        return FALSE;
    }
    db->addColumn(cindex, cname, ctype, cdefault, cid);
    columnMap.insert(cindex, cid);
    return TRUE;
}

bool XMLImport::addView()
{
    QString vname = getField("vname");
    int vrpp = getField("vrpp").toInt();
    int vdeskrpp = getField("vdeskrpp").toInt();
    if (error != "") {
        return FALSE;
    }
    if (vname != "_all") {
        if (!validateName(vname)) {
            return FALSE;
        }
    }
    if (indexMap.contains(vname)) {
        error = QObject::tr("Duplicate") + " vname: " + vname;
        return FALSE;
    }
    QMap<int,QString> mapping;
    indexMap.insert(vname, mapping);
    // empty column list; view columns will be added later
    QStringList cols;
    db->addView(vname, cols, vrpp, vdeskrpp);
    return TRUE;
}

bool XMLImport::addViewColumn()
{
    QString vcview = getField("vcview");
    int vcindex = getField("vcindex").toInt();
    QString vcname = getField("vcname");
    int vcwidth = getField("vcwidth").toInt();
    int vcdeskwidth = getField("vcdeskwidth").toInt();
    if (error != "") {
        return FALSE;
    }
    if (!indexMap.contains(vcview)) {
        error = QObject::tr("Invalid") + " vcview: " + vcview;
        return FALSE;
    }
    if (colNames.findIndex(vcname) == -1) {
        error = QObject::tr("Invalid") + " vcname: " + vcname;
        return FALSE;
    }
    indexMap[vcview].insert(vcindex, vcname);
    db->addViewColumn(vcview, vcname, vcindex, vcwidth, vcdeskwidth);
    return TRUE;
}

bool XMLImport::addSort()
{
    QString sname = getField("sname");
    if (error != "") {
        return FALSE;
    }
    if (sname != "_single") {
        if (!validateName(sname)) {
            return FALSE;
        }
    }
    if (indexMap.contains(sname)) {
        error = QObject::tr("Duplicate") + " sname: " + sname;
        return FALSE;
    }
    QMap<int,QString> mapping;
    indexMap.insert(sname, mapping);
    return TRUE;
}

bool XMLImport::addSortColumn()
{
    QString scsort = getField("scsort");
    int scindex = getField("scindex").toInt();
    QString scname = getField("scname");
    int scdesc = getField("scdesc").toInt();
    if (error != "") {
        return FALSE;
    }
    if (!indexMap.contains(scsort)) {
        error = QObject::tr("Invalid") + " scsort: " + scsort;
        return FALSE;
    }
    if (colNames.findIndex(scname) == -1) {
        error = QObject::tr("Invalid") + " scname: " + scname;
        return FALSE;
    }
    if (scdesc == 1) {
        // mangle name to indicate descending order
        scname = "_" + scname;
    }
    indexMap[scsort].insert(scindex, scname);
    return TRUE;
}

bool XMLImport::addFilter()
{
    QString fname = getField("fname");
    if (error != "") {
        return FALSE;
    }
    if (fname != "_allrows" && fname != "_simple") {
        if (!validateName(fname)) {
            return FALSE;
        }
    }
    if (indexMap.contains(fname)) {
        error = QObject::tr("Duplicate") + " fname: " + fname;
        return FALSE;
    }
    QMap<int,QString> mapping;
    indexMap.insert(fname, mapping);
    Filter *filter = new Filter(db, fname);
    filterMap.insert(fname, filter);
    return TRUE;
}

bool XMLImport::addFilterCondition()
{
    QString fcfilter = getField("fcfilter");
    int fcposition = getField("fcposition").toInt();
    QString fccolumn = getField("fccolumn");
    int fcoperator = getField("fcoperator").toInt();
    QString fcconstant = getField("fcconstant");
    int fccase = getField("fccase").toInt();
    if (error != "") {
        return FALSE;
    }
    if (!indexMap.contains(fcfilter)) {
        error = QObject::tr("Invalid") + " fcfilter: " + fcfilter;
        return FALSE;
    }
    int ctype = STRING;
    if (fccolumn != "_anytext") {
        if (colNames.findIndex(fccolumn) == -1) {
            error = QObject::tr("Invalid") + " fccolumn: " + fccolumn;
            return FALSE;
        }
        ctype = db->getType(fccolumn);
        QString errMsg = db->isValidValue(ctype, fcconstant);
        if (errMsg != "") {
            error = QObject::tr("Invalid") + " fcconstant: " + fcconstant + "("
                    + errMsg + ")";
            return FALSE;
        }
    }
    if (!isValidOperator(ctype, fcoperator, fccase)) {
        return FALSE;
    }
    indexMap[fcfilter].insert(fcposition, QString::number(fcposition));
    Condition *condition = new Condition(db);
    condition->setColName(fccolumn);
    condition->setOperator(fcoperator);
    condition->setConstant(fcconstant);
    condition->setCaseSensitive(fccase);
    filterMap[fcfilter]->addCondition(condition, fcposition);
    return TRUE;
}

bool XMLImport::addRow()
{
    QStringList values;
    for (int i = 0; i < colCount; i++) {
        values.append(getDataField(idList[i]));
    }
    if (error != "") {
        return FALSE;
    }
    error = db->addRow(values);
    return (error == "");
}

bool XMLImport::validateName(const QString &name)
{
    if (name.isEmpty()) {
        error = QObject::tr("No name entered");
        return FALSE;
    }
    if (name.startsWith("_")) {
        error = QObject::tr("Name must not start with '_'") + ": " + name;
        return FALSE;
    }
    return TRUE;
}

bool XMLImport::validateGlobal()
{
    if (gview != "_all") {
        QStringList views = db->listViews();
        if (views.findIndex(gview) == -1) {
            error = QObject::tr("Invalid") + " gview: " + gview;
            return FALSE;
        }
    }
    if (gsort != "") {
        QStringList sortings = db->listSortings();
        if (sortings.findIndex(gsort) == -1) {
            error = QObject::tr("Invalid") + " gsort: " + gsort;
            return FALSE;
        }
    }
    if (gfilter != "_allrows") {
        QStringList filters = db->listFilters();
        if (filters.findIndex(gfilter) == -1) {
            error = QObject::tr("Invalid") + " gfilter: " + gfilter;
            return FALSE;
        }
    }
    return TRUE;
}

bool XMLImport::validateEnums()
{
    int count = enumMap.count();
    for (int i = 0; i < count; i++) {
        if (!enumMap.contains(i)) {
            error = QObject::tr("Missing") + " eindex: " + QString::number(i);
            return FALSE;
        }
        enumIds.append(enumMap[i]);
    }
    if (containsDuplicate(enumIds, "eid")) {
        return FALSE;
    }
    return TRUE;
}

bool XMLImport::validateColumns()
{
    colNames = db->listColumns();
    if (containsDuplicate(colNames, "cname")) {
        return FALSE;
    }
    colCount = columnMap.count();
    for (int i = 0; i < colCount; i++) {
        if (!columnMap.contains(i)) {
            error = QObject::tr("Missing") + " cindex: " + QString::number(i);
            return FALSE;
        }
        idList.append(columnMap[i]);
    }
    if (containsDuplicate(idList, "cid")) {
        return FALSE;
    }
    db->updateDataFormat();
    return TRUE;
}

bool XMLImport::validateViewColumns()
{
    if (!indexMap.contains("_all")) {
        error = QObject::tr("Missing") + " vcview: _all";
        return FALSE;
    }
    if (!validateIndexMap("vcindex", "vcname")) {
        return FALSE;
    }
    return TRUE;
}

bool XMLImport::validateSortColumns()
{
    if (!validateIndexMap("scindex", "scname")) {
        return FALSE;
    }
    IndexMap::Iterator iter;
    QStringList allCols;
    QStringList descCols;
    int count;
    for (iter = indexMap.begin(); iter != indexMap.end(); ++iter) {
        allCols.clear();
        descCols.clear();
        QString sname = iter.key();
        QMap<int,QString> mapping = iter.data();
        count = mapping.count();
        for (int i = 0; i < count; i++) {
            QString item = mapping[i];
            if (item.startsWith("_")) {
                item = item.remove(0, 1);
                descCols.append(item);
            }
            allCols.append(item);
        }
        db->addSorting(sname, allCols, descCols);
    }
    return TRUE;
}

bool XMLImport::validateFilterConditions()
{
    // validation of global table data must happen after everything is defined
    if (!validateGlobal()) {
        return FALSE;
    }
    if (!validateIndexMap("fcposition", "fcposition")) {
        return FALSE;
    }
    FilterMap::Iterator iter;
    for (iter = filterMap.begin(); iter != filterMap.end(); ++iter) {
        Filter *filter = iter.data();
        if (iter.key() != "_allrows") {
            // _allrows already added during database initialization
            db->addFilter(filter, FALSE);
        }
        delete filter;
    }
    return TRUE;
}

bool XMLImport::isValidDefault(const QString &cname, int ctype,
                               const QString &cdefault)
{
    // in case it's needed...
    QString errorMsg = QObject::tr("Invalid") + " cdefault: " + cdefault
                       + " (" + cname + ")";
    if (ctype == DATE) {
        bool ok = FALSE;
        int value = cdefault.toInt(&ok);
        if (!ok || (value != 0 && value != 17520914)) {
            error = errorMsg;
            return FALSE;
        }
    }
    else if (ctype == TIME) {
        bool ok = FALSE;
        int value = cdefault.toInt(&ok);
        if (!ok || (value != 0 && value != -1)) {
            error = errorMsg;
            return FALSE;
        }
    }
    else if (ctype >= FIRST_ENUM) {
        QStringList options = db->listEnumOptions(ctype);
        if (options.findIndex(cdefault) == -1) {
            error = errorMsg;
            return FALSE;
        }
    }
    else {
        if (db->isValidValue(ctype, cdefault) != "") {
            error = errorMsg;
            return FALSE;
        }
    }
    return TRUE;
}

bool XMLImport::isValidOperator(int type, int op, int cs)
{
    // in case it's needed...
    QString opError = QObject::tr("Invalid") + " fcoperator: "
            + QString::number(op) + " (ctype = " + QString::number(type) + ")";
    if (type == BOOLEAN) {
        if (op != EQUALS) {
            error = opError;
            return FALSE;
        }
    }
    else if (type >= FIRST_ENUM) {
        if (op != EQUALS && op != NOTEQUAL) {
            error = opError;
            return FALSE;
        }
        if (cs == 0) {
            error = QObject::tr("Invalid") + " fccase: 0 (ctype = "
                    + QString::number(type) + ")";

        }
    }
    else if (type == STRING || type == NOTE) {
        if (op != EQUALS && op != CONTAINS && op != STARTSWITH
                && op != NOTEQUAL) {
            error = opError;
            return FALSE;
        }
    }
    else {
        if (op != EQUALS && op != NOTEQUAL && op != LESSTHAN && op != LESSEQUAL
                && op != GREATERTHAN && op != GREATEREQUAL) {
            error = opError;
            return FALSE;
        }
    }
    return TRUE;
}

bool XMLImport::containsDuplicate(const QStringList &names,
                                  const QString &element)
{
    QStringList copy(names);
    QStringList::Iterator iter = copy.begin();
    int count = copy.count();
    for (int i = 0; i < count; i++) {
        QString name = *iter;
        iter = copy.remove(iter);
        if (copy.findIndex(name) != -1) {
            error = QObject::tr("Duplicate") + " " + element + ": " + name;
            return TRUE;
        }
    }
    return FALSE;
}

bool XMLImport::containsDuplicate(const IntList &items, const QString &element)
{
    IntList copy(items);
    IntList::Iterator iter = copy.begin();
    int count = copy.count();
    for (int i = 0; i < count; i++) {
        int item = *iter;
        iter = copy.remove(iter);
        if (copy.findIndex(item) != -1) {
            error = QObject::tr("Duplicate") + " " + element + ": "
                    + QString::number(item);
            return TRUE;
        }
    }
    return FALSE;
}

bool XMLImport::validateIndexMap(const QString &indexElement,
                                 const QString &stringElement)
{
    IndexMap::Iterator iter;
    int i;
    QStringList itemList;
    for (iter = indexMap.begin(); iter != indexMap.end(); ++iter) {
        QMap<int,QString> mapping = iter.data();
        int indexCount = mapping.count();
        itemList.clear();
        for (i = 0; i < indexCount; i++) {
            if (!mapping.contains(i)) {
                error = QObject::tr("Missing") + " " + indexElement + ": "
                        + QString::number(i);
                return FALSE;
            }
            QString item = mapping[i];
            if (item.startsWith("_")) {
                // unmangle descending sort columns
                item = item.remove(0, 1);
            }
            itemList.append(item);
        }
        if (iter.key() == "_all") {
            if (itemList != colNames) {
                error = QObject::tr("Incorrect _all view column sequence");
            }
        }
        if (containsDuplicate(itemList, stringElement)) {
            return FALSE;
        }
    }
    return TRUE;
}

bool XMLImport::setField(const QString &name)
{
    if (containers.findIndex(parent) == -1) {
        // no meaningful text content, move on
        return TRUE;
    }
    if (booleans.contains(name)) {
        bool ok = FALSE;
        int value = text.toInt(&ok);
        if (!ok || value < 0 || value > 1) {
            error = name + ": " + QObject::tr("must be 0 or 1");
            return FALSE;
        }
    }
    else if (integers.contains(name)) {
        bool ok = FALSE;
        text.toInt(&ok);
        if (!ok) {
            error = name + ": " + QObject::tr("must be an integer");
            return FALSE;
        }
    }
    else if (nonNegativeIntegers.contains(name)) {
        bool ok = FALSE;
        int value = text.toInt(&ok);
        if (!ok || value < 0) {
            error = name + ": "
                    + QObject::tr("must be a non-negative integer");
            return FALSE;
        }
    }
    else if (positiveIntegers.contains(name)) {
        bool ok = FALSE;
        int value = text.toInt(&ok);
        if (!ok || value <= 0) {
            error = name + ": " + QObject::tr("must be a positive integer");
            return FALSE;
        }
    }
    if (parent != "r" && fields.contains(name)) {
        error = QObject::tr("Duplicate element") + ": " + name;
        return FALSE;
    }
    fields.insert(name, text);
    return TRUE;
}

QString XMLImport::getField(const QString &name)
{
    if (!fields.contains(name)) {
        error = QObject::tr("Missing element") + ": " + name;
        return "";
    }
    return fields[name];
}

QString XMLImport::getDataField(int columnId)
{
    QString idString = QString::number(columnId);
    if (!fields.contains(idString)) {
        error = QObject::tr("Missing data for column ID") + ": " + idString;
        return "";
    }
    return fields[idString];
}

void XMLImport::buildParentsMap()
{
    parents.insert("global", "portabase");
    parents.insert("columns", "portabase");
    parents.insert("views", "portabase");
    parents.insert("viewcolumns", "portabase");
    parents.insert("sorts", "portabase");
    parents.insert("sortcolumns", "portabase");
    parents.insert("filters", "portabase");
    parents.insert("filterconditions", "portabase");
    parents.insert("enums", "portabase");
    parents.insert("enumoptions", "portabase");
    parents.insert("data", "portabase");

    parents.insert("gversion", "global");
    parents.insert("gview", "global");
    parents.insert("gsort", "global");
    parents.insert("gfilter", "global");

    parents.insert("enum", "enums");
    parents.insert("eindex", "enum");
    parents.insert("ename", "enum");
    parents.insert("eid", "enum");

    parents.insert("enumoption", "enumoptions");
    parents.insert("eoenum", "enumoption");
    parents.insert("eoindex", "enumoption");
    parents.insert("eotext", "enumoption");

    parents.insert("column", "columns");
    parents.insert("cindex", "column");
    parents.insert("cname", "column");
    parents.insert("ctype", "column");
    parents.insert("cdefault", "column");
    parents.insert("cid", "column");

    parents.insert("view", "views");
    parents.insert("vname", "view");
    parents.insert("vrpp", "view");
    parents.insert("vdeskrpp", "view");

    parents.insert("viewcolumn", "viewcolumns");
    parents.insert("vcview", "viewcolumn");
    parents.insert("vcindex", "viewcolumn");
    parents.insert("vcname", "viewcolumn");
    parents.insert("vcwidth", "viewcolumn");
    parents.insert("vcdeskwidth", "viewcolumn");

    parents.insert("sort", "sorts");
    parents.insert("sname", "sort");

    parents.insert("sortcolumn", "sortcolumns");
    parents.insert("scsort", "sortcolumn");
    parents.insert("scindex", "sortcolumn");
    parents.insert("scname", "sortcolumn");
    parents.insert("scdesc", "sortcolumn");

    parents.insert("filter", "filters");
    parents.insert("fname", "filter");

    parents.insert("filtercondition", "filterconditions");
    parents.insert("fcfilter", "filtercondition");
    parents.insert("fcposition", "filtercondition");
    parents.insert("fccolumn", "filtercondition");
    parents.insert("fcoperator", "filtercondition");
    parents.insert("fcconstant", "filtercondition");
    parents.insert("fccase", "filtercondition");

    parents.insert("r", "data");
    parents.insert("s", "r");
    parents.insert("i", "r");
    parents.insert("f", "r");
    parents.insert("b", "r");
    parents.insert("n", "r");
    parents.insert("d", "r");
    parents.insert("t", "r");
    parents.insert("e", "r");
}
