/*
 * xmlimport.cpp
 *
 * (c) 2003,2009-2010,2015-2016 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file xmlimport.cpp
 * Source file for XMLImport
 */

#include <QLocale>
#include "calc/calcnode.h"
#include "condition.h"
#include "database.h"
#include "filter.h"
#include "formatting.h"
#include "pbdialog.h"
#include "xmlimport.h"

/**
 * Constructor.
 *
 * @param dbase The database being imported into
 */
XMLImport::XMLImport(Database *dbase) : QObject(), QXmlDefaultHandler(),
  db(dbase), error("")
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
    sections.append("calcs");
    sections.append("calcnodes");
    sections.append("data");

    optionalSections.append("calcs");
    optionalSections.append("calcnodes");

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
    containers.append("calc");
    containers.append("calcnode");
    containers.append("r");

    dataFields.append("s");
    dataFields.append("i");
    dataFields.append("f");
    dataFields.append("b");
    dataFields.append("n");
    dataFields.append("d");
    dataFields.append("t");
    dataFields.append("c");
    dataFields.append("q");
    dataFields.append("p");
    dataFields.append("e");

    booleans.append("scdesc");
    booleans.append("fccase");
    booleans.append("b");

    integers.append("i");
    integers.append("d");
    integers.append("t");
    integers.append("q");

    nonNegativeIntegers.append("eindex");
    nonNegativeIntegers.append("eoindex");
    nonNegativeIntegers.append("cindex");
    nonNegativeIntegers.append("ctype");
    nonNegativeIntegers.append("cid");
    nonNegativeIntegers.append("vcindex");
    nonNegativeIntegers.append("scindex");
    nonNegativeIntegers.append("fcposition");
    nonNegativeIntegers.append("fcoperator");
    nonNegativeIntegers.append("calcid");
    nonNegativeIntegers.append("calcdecimals");
    nonNegativeIntegers.append("cnid");
    nonNegativeIntegers.append("cnnodeid");
    nonNegativeIntegers.append("cnparentid");
    nonNegativeIntegers.append("cntype");

    positiveIntegers.append("gversion");
    positiveIntegers.append("eid");
    positiveIntegers.append("vrpp");
    positiveIntegers.append("vdeskrpp");
    positiveIntegers.append("vcwidth");
    positiveIntegers.append("vcdeskwidth");
}

/**
 * Get the base error message (if any) generated during the import attempt.
 *
 * @return An error message, or an empty string if none
 */
QString XMLImport::errorString() const
{
    return error;
}

/**
 * Get a detailed error message, including the line and column of the XML
 * file at which the error occurred.
 *
 * @return A detailed error message, or an empty string if none
 */
QString XMLImport::formattedError()
{
    if (!error.isEmpty()) {
        QLocale locale = QLocale::system();
        QString fullError = tr("Error at") + ": ";
        fullError += tr("Line") + " ";
        fullError += locale.toString(xmlLocator->lineNumber()) + ", ";
        fullError += tr("Column") + " ";
        fullError += locale.toString(xmlLocator->columnNumber()) + "\n";
        fullError += error;
        return fullError;
    }
    return error;
}

/**
 * Called when the SAX parser starts work on a new XML file.  Resets a number
 * of string and collection attributes back to their default, empty state.
 *
 * @return True if the state attributes were successfully reset
 */
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
    return true;
}

/**
 * Called when the SAX parser finishes parsing the start tag of an element.
 * Makes sure that this element can validly be positioned here, and prepares
 * to handle any contained text and/or attributes.
 *
 * @param namespaceURI Empty string, since no namespace processing is being done
 * @param localName Empty string, since no namespace processing is being done
 * @param qName The name of the element being parsed
 * @param atts The attributes of the element being parsed
 * @return True if the element looks valid so far, false otherwise
 */
bool XMLImport::startElement(const QString&, const QString&,
                             const QString &qName, const QXmlAttributes &atts)
{
    int count = ancestors.count();
    if (qName == "portabase") {
        if (count == 0) {
            ancestors.append(qName);
            return true;
        }
        else {
            error = "portabase " + tr("only allowed as root element");
            return false;
        }
    }
    if (!parents.contains(qName)) {
        error = tr("Unknown element") + ": " + qName;
        return false;
    }
    parent = ancestors[count - 1];
    if (parents[qName] != parent) {
        error = tr("%1 not allowed as child of %2");
        error = error.arg(qName).arg(parent);
        return false;
    }
    int sectionIndex = sections.indexOf(qName);
    if (sectionIndex != -1) {
        QString missing = missingSection(lastSection, sectionIndex);
        if (!missing.isEmpty()) {
            error = tr("Missing element") + ": " + missing;
            return false;
        }
        lastSection = qName;
    }
    ancestors.append(qName);
    if (dataFields.contains(qName)) {
        bool ok = false;
        colId = atts.value("c");
        colId.toInt(&ok);
        if (!ok) {
            error = tr("\"c\" attribute is missing or non-integer");
            return false;
        }
    }
    if (containers.contains(qName)) {
        fields.clear();
    }
    if (qName == "enums" || qName == "views" || qName == "sorts"
            || qName == "filters" || qName == "calcnodes") {
        indexMap.clear();
    }
    text = "";
    return true;
}

/**
 * Called when the SAX parser finishes parsing the end tag of an element.
 * Adds the information gathered over the course of parsing the element to
 * the target PortaBase file.
 *
 * @param namespaceURI Empty string, since no namespace processing is being done
 * @param localName Empty string, since no namespace processing is being done
 * @param qName The name of the element being parsed
 * @return True if the element contained valid data, false otherwise
 */
bool XMLImport::endElement(const QString&, const QString&,
                           const QString &qName)
{
    ancestors.removeAll(qName);
    if (!setField(qName)) {
        return false;
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
    else if (qName == "calc") {
        return addCalc();
    }
    else if (qName == "calcnode") {
        return addCalcNode();
    }
    else if (qName == "calcnodes") {
        return validateCalcNodes();
    }
    else if (qName == "r") {
        return addRow();
    }
    else if (qName == "data") {
        db->calculateAll();
        return true;
    }
    else if (dataFields.contains(qName)) {
        fields.insert(colId, text);
        return true;
    }
    else {
        fields.insert(qName, text);
        return true;
    }
}

/**
 * Called when text content is found in the body of an element.  Appends it
 * to any text previously found in the same element, and saves it for later
 * use when parsing the element's end tag.
 *
 * @param ch The text which was just parsed
 * @return True if the text was appended successfully
 */
bool XMLImport::characters(const QString &ch)
{
    text += ch;
    return true;
}

/**
 * Called when the SAX parser encounters an error from which it cannot recover
 * (typically, this means the file being parsed is not a well-formed XML
 * file).  Stores the exception message as the error message and returns
 * false.
 *
 * @param exception The SAX parsing exception which occurred
 * @return False (always)
 */
bool XMLImport::fatalError(const QXmlParseException &exception)
{
    if (error.isEmpty()) {
        error = exception.message();
    }
    return false;
}

/**
 * Called by the SAX parser to provide this object with a means of determining
 * exactly where in the source XML file the parser is.  This information is
 * used to give line and column numbers in error messages.
 *
 * @param locator The object used to determine the parser's current location
 *                in the file
 */
void XMLImport::setDocumentLocator(QXmlLocator *locator)
{
    xmlLocator = locator;
}

/**
 * Check to see if any of the top-level elements in the file have been
 * omitted, and if so if that omission can still result in a valid file.
 * (Some sections are in fact optional, due to having been introduced in
 * newer versions of PortaBase so exports from older versions won't have
 * them.)
 *
 * @param before The name of the last top-level element which was parsed
 * @param afterIndex The position index at which the top-level element now
 *                   being parsed is found when all sections are present
 * @return The name of a missing required element, or an empty string if none
 */
QString XMLImport::missingSection(const QString &before, int afterIndex)
{
    if (afterIndex == 0) {
        // first section, there can't be one missing before it
        return "";
    }
    QString previous = sections[afterIndex - 1];
    if (before == previous) {
        // no missing section
        return "";
    }
    if (!optionalSections.contains(previous)) {
        // missing non-optional section
        return before;
    }
    // missing an optional section, see if anything earlier is missing
    return missingSection(before, afterIndex - 1);
}

/**
 * Update the "_global" view in the PortaBase file to contain the data which
 * was found in the XML file (specifically, the current view, filter, and
 * sorting names).  A specified file format version older than the first
 * PortaBase version to support XML export or newer than the one native to
 * the version of PortaBase being run is treated as an error.  The created
 * file will always be of the version native for the PortaBase binary in use.
 *
 * @return False if an error occurred, true otherwise
 */
bool XMLImport::updateGlobalRecord()
{
    gversion = getField("gversion").toInt();
    gview = getField("gview");
    gsort = getField("gsort");
    gfilter = getField("gfilter");
    if (!error.isEmpty()) {
        return false;
    }
    if (gversion < 7 || gversion > FILE_VERSION) {
        error = tr("Unsupported") + " gversion: " + QString::number(gversion);
        return false;
    }
    db->setGlobalInfo(gview, gsort, gfilter);
    return true;
}

/**
 * Add the enumeration which was just parsed from the XML file to the
 * target database.  None of its options are added here, those will be found
 * later in the XML file and added to the database then.
 *
 * @return False if an error occurred, true otherwise
 */
bool XMLImport::addEnum()
{
    int eindex = getField("eindex").toInt();
    QString ename = getField("ename");
    int eid = getField("eid").toInt();
    if (!error.isEmpty()) {
        return false;
    }
    if (!validateName(ename)) {
        return false;
    }
    if (eid < FIRST_ENUM) {
        error = tr("Invalid") + " eid: " + QString::number(eid);
        return false;
    }
    if (indexMap.contains(ename)) {
        error = tr("Duplicate") + " ename: " + ename;
        return false;
    }
    QMap<int,QString> mapping;
    indexMap.insert(ename, mapping);
    // empty option list; enum options will be added later
    QStringList options;
    db->addEnum(ename, options, eindex, eid);
    enumMap.insert(eindex, eid);
    return true;
}

/**
 * Add the enumeration option which was just parsed from the XML file to the
 * target database.
 *
 * @return False if an error occurred, true otherwise
 */
bool XMLImport::addEnumOption()
{
    QString idString = getField("eoenum");
    int eoenum = idString.toInt();
    int eoindex = getField("eoindex").toInt();
    QString eotext = getField("eotext");
    if (!error.isEmpty()) {
        return false;
    }
    if (!validateName(eotext)) {
        return false;
    }
    if (!enumIds.contains(eoenum)) {
        error = tr("Invalid") + " eoenum: " + idString;
        return false;
    }
    indexMap[idString].insert(eoindex, eotext);
    db->addEnumOption(db->getEnumName(eoenum), eotext, eoindex);
    return true;
}

/**
 * Add the column definition which was just parsed from the XML file to the
 * target database.
 *
 * @return False if an error occurred, true otherwise
 */
bool XMLImport::addColumn()
{
    int cindex = getField("cindex").toInt();
    QString cname = getField("cname");
    int ctype = getField("ctype").toInt();
    QString cdefault = getField("cdefault");
    int cid = getField("cid").toInt();
    if (!error.isEmpty()) {
        return false;
    }
    if (!validateName(cname)) {
        return false;
    }
    if (ctype > LAST_TYPE) {
        if (!enumIds.contains(ctype)) {
            error = tr("Invalid") + " ctype: " + QString::number(ctype);
            return false;
        }
    }
    if (!isValidDefault(cname, ctype, cdefault)) {
        return false;
    }
    db->addColumn(cindex, cname, ctype, cdefault, cid);
    columnMap.insert(cindex, cid);
    return true;
}

/**
 * Add the view which was just parsed from the XML file to the target
 * database.  None of its component columns are added here, those will be
 * found later in the XML file and added to the database then.
 *
 * @return False if an error occurred, true otherwise
 */
bool XMLImport::addView()
{
    QString vname = getField("vname");
    int vrpp = getField("vrpp").toInt();
    int vdeskrpp = getField("vdeskrpp").toInt();
    if (!error.isEmpty()) {
        return false;
    }
    QString defaultSort = getOptionalField("vsort", "_none");
    QString defaultFilter = getOptionalField("vfilter", "_none");
    if (vname != "_all") {
        if (!validateName(vname)) {
            return false;
        }
    }
    if (indexMap.contains(vname)) {
        error = tr("Duplicate") + " vname: " + vname;
        return false;
    }
    QMap<int,QString> mapping;
    indexMap.insert(vname, mapping);
    // empty column list; view columns will be added later
    QStringList cols;
    db->addView(vname, cols, defaultSort, defaultFilter, vrpp, vdeskrpp);
    return true;
}

/**
 * Add the view column definition which was just parsed from the XML file to
 * the target database.
 *
 * @return False if an error occurred, true otherwise
 */
bool XMLImport::addViewColumn()
{
    QString vcview = getField("vcview");
    int vcindex = getField("vcindex").toInt();
    QString vcname = getField("vcname");
    int vcwidth = getField("vcwidth").toInt();
    int vcdeskwidth = getField("vcdeskwidth").toInt();
    if (!error.isEmpty()) {
        return false;
    }
    if (!indexMap.contains(vcview)) {
        error = tr("Invalid") + " vcview: " + vcview;
        return false;
    }
    if (!colNames.contains(vcname)) {
        error = tr("Invalid") + " vcname: " + vcname;
        return false;
    }
    indexMap[vcview].insert(vcindex, vcname);
    db->addViewColumn(vcview, vcname, vcindex, vcwidth, vcdeskwidth);
    return true;
}

/**
 * Add the sorting which was just parsed from the XML file to a collection of
 * sorting data.  It will be added to the database later, when parsing of the
 * &lt;sortcolumns&gt; element concludes.
 *
 * @return False if an error occurred, true otherwise
 */
bool XMLImport::addSort()
{
    QString sname = getField("sname");
    if (!error.isEmpty()) {
        return false;
    }
    if (sname != "_single") {
        if (!validateName(sname)) {
            return false;
        }
    }
    if (indexMap.contains(sname)) {
        error = tr("Duplicate") + " sname: " + sname;
        return false;
    }
    QMap<int,QString> mapping;
    indexMap.insert(sname, mapping);
    return true;
}

/**
 * Add the sorting column definition which was just parsed from the XML file
 * to the collection of that sorting's columns.  These will be added to the
 * database later, when parsing of the &lt;sortcolumns&gt; element concludes.
 *
 * @return False if an error occurred, true otherwise
 */
bool XMLImport::addSortColumn()
{
    QString scsort = getField("scsort");
    int scindex = getField("scindex").toInt();
    QString scname = getField("scname");
    int scdesc = getField("scdesc").toInt();
    if (!error.isEmpty()) {
        return false;
    }
    if (!indexMap.contains(scsort)) {
        error = tr("Invalid") + " scsort: " + scsort;
        return false;
    }
    if (!colNames.contains(scname)) {
        error = tr("Invalid") + " scname: " + scname;
        return false;
    }
    if (scdesc == 1) {
        // mangle name to indicate descending order
        scname = "_" + scname;
    }
    indexMap[scsort].insert(scindex, scname);
    return true;
}

/**
 * Add the filter which was just parsed from the XML file to a collection of
 * filter data.  It will be added to the database later, when parsing of the
 * &lt;filterconditions&gt; element concludes.
 *
 * @return False if an error occurred, true otherwise
 */
bool XMLImport::addFilter()
{
    QString fname = getField("fname");
    if (!error.isEmpty()) {
        return false;
    }
    if (fname != "_allrows" && fname != "_simple") {
        if (!validateName(fname)) {
            return false;
        }
    }
    if (indexMap.contains(fname)) {
        error = tr("Duplicate") + " fname: " + fname;
        return false;
    }
    QMap<int,QString> mapping;
    indexMap.insert(fname, mapping);
    Filter *filter = new Filter(db, fname);
    filterMap.insert(fname, filter);
    return true;
}

/**
 * Add the filter condition definition which was just parsed from the XML file
 * to the collection of that filter's conditions.  These will be added to the
 * database later, when parsing of the &lt;filterconditions&gt; element concludes.
 *
 * @return False if an error occurred, true otherwise
 */
bool XMLImport::addFilterCondition()
{
    QString fcfilter = getField("fcfilter");
    int fcposition = getField("fcposition").toInt();
    QString fccolumn = getField("fccolumn");
    int fcoperator = getField("fcoperator").toInt();
    QString fcconstant = getField("fcconstant");
    int fccase = getField("fccase").toInt();
    if (!error.isEmpty()) {
        return false;
    }
    if (!indexMap.contains(fcfilter)) {
        error = tr("Invalid") + " fcfilter: " + fcfilter;
        return false;
    }
    int ctype = STRING;
    if (fccolumn != "_anytext") {
        if (!colNames.contains(fccolumn)) {
            error = tr("Invalid") + " fccolumn: " + fccolumn;
            return false;
        }
        ctype = db->getType(fccolumn);
        QString errMsg = db->isValidValue(ctype, fcconstant);
        if (!errMsg.isEmpty()) {
            error = tr("Invalid") + " fcconstant: " + fcconstant + "("
                    + errMsg + ")";
            return false;
        }
    }
    if (!isValidOperator(ctype, fcoperator, fccase)) {
        return false;
    }
    indexMap[fcfilter].insert(fcposition, QString::number(fcposition));
    Condition *condition = new Condition(db);
    condition->setColName(fccolumn);
    condition->setOperator((Condition::Operator)fcoperator);
    condition->setConstant(fcconstant);
    condition->setCaseSensitive(fccase);
    filterMap[fcfilter]->addCondition(condition, fcposition);
    return true;
}

/**
 * Add the calculation which was just parsed from the XML file to the target
 * database.  None of its component nodes are added here, those will be
 * found later in the XML file and added to the database then.
 *
 * @return False if an error occurred, true otherwise
 */
bool XMLImport::addCalc()
{
    int calcid = getField("calcid").toInt();
    int calcdecimals = getField("calcdecimals").toInt();
    if (!error.isEmpty()) {
        return false;
    }
    int index = idList.indexOf(calcid);
    if (index == -1) {
        error = tr("Invalid") + " calcid: " + QString::number(calcid);
        return false;
    }
    QString colName = colNames[index];
    int type = db->getType(colName);
    if (type != CALC) {
        error = tr("Invalid") + " calcid: " + QString::number(calcid);
        return false;
    }
    db->updateCalc(colName, 0, calcdecimals);
    return true;
}

/**
 * Add the calculation node definition which was just parsed from the XML
 * file to the target database.
 *
 * @return False if an error occurred, true otherwise
 */
bool XMLImport::addCalcNode()
{
    int cnid = getField("cnid").toInt();
    int cnnodeid = getField("cnnodeid").toInt();
    int cnparentid = getField("cnparentid").toInt();
    CalcNode::NodeType cntype = (CalcNode::NodeType)getField("cntype").toInt();
    QString cnvalue = getField("cnvalue");
    if (!error.isEmpty()) {
        return false;
    }
    int index = idList.indexOf(cnid);
    if (index == -1) {
        error = tr("Invalid") + " cnid: " + QString::number(cnid);
        return false;
    }
    QString colName = colNames[index];
    int type = db->getType(colName);
    if (type != CALC) {
        error = tr("Invalid") + " cnid: " + QString::number(cnid);
        return false;
    }
    if (cnparentid >= cnnodeid && !(cnparentid == 0 && cnnodeid == 0)) {
        error = tr("Invalid") + " cnparentid: " + QString::number(cnparentid);
        return false;
    }
    if (!indexMap.contains(colName)) {
        QMap<int,QString> mapping;
        indexMap.insert(colName, mapping);
    }
    indexMap[colName].insert(cnnodeid, QString::number(cnparentid));
    CalcNode node(cntype, cnvalue);
    db->addCalcNode(cnid, &node, cnnodeid, cnparentid);
    return true;
}

/**
 * Add the row of the main data table which was just parsed from the XML
 * file to the target database.
 *
 * @return False if an error occurred, true otherwise
 */
bool XMLImport::addRow()
{
    QStringList values;
    for (int i = 0; i < colCount; i++) {
        values.append(getDataField(idList[i]));
    }
    if (!error.isEmpty()) {
        return false;
    }
    error = db->addRow(values, 0, true);
    return error.isEmpty();
}

/**
 * Make sure that the provided name meets PortaBase's minimum standards for
 * database artifact names: not empty, and doesn't start with an underscore
 * (names with leading underscores are reserved for internal use).
 *
 * @param name The name to be validated
 * @return True if the provided name is potentially valid, false otherwise
 */
bool XMLImport::validateName(const QString &name)
{
    if (name.isEmpty()) {
        error = PBDialog::tr("No name entered");
        return false;
    }
    if (name.startsWith("_")) {
        error = PBDialog::tr("Name must not start with '_'") + ": " + name;
        return false;
    }
    return true;
}

/**
 * Make sure that the data intended for the "_global" view actually referred
 * to items present later in the file.  Called after all views, sortings, and
 * filters have been parsed and imported.
 *
 * @return False if a missing view, sorting, or filter is referenced
 */
bool XMLImport::validateGlobal()
{
    if (gview != "_all") {
        QStringList views = db->listViews();
        if (!views.contains(gview)) {
            error = tr("Invalid") + " gview: " + gview;
            return false;
        }
    }
    if (!gsort.isEmpty()) {
        QStringList sortings = db->listSortings();
        if (!sortings.contains(gsort)) {
            error = tr("Invalid") + " gsort: " + gsort;
            return false;
        }
    }
    if (gfilter != "_allrows") {
        QStringList filters = db->listFilters();
        if (!filters.contains(gfilter)) {
            error = tr("Invalid") + " gfilter: " + gfilter;
            return false;
        }
    }
    return true;
}

/**
 * Make sure that the set of enums presented in the XML is valid when taken
 * as a whole.  Returns false if there is a gap in the position index
 * sequence or if an ID is used more than once.  Called when parsing the
 * &lt;enums&gt; element's closing tag.
 *
 * @return False if a problem was found with the enum set, true otherwise
 */
bool XMLImport::validateEnums()
{
    int count = enumMap.count();
    for (int i = 0; i < count; i++) {
        if (!enumMap.contains(i)) {
            error = tr("Missing") + " eindex: " + QString::number(i);
            return false;
        }
        enumIds.append(enumMap[i]);
    }
    if (containsDuplicate(enumIds, "eid")) {
        return false;
    }
    return true;
}

/**
 * Make sure that the set of data column definitions is valid when taken as a
 * whole.  Returns false if there is a gap in the position index sequence or
 * if a name or ID is used more than once.  Otherwise, initializes the data
 * table with the columns that were in the XML file.
 *
 * @return False if the provided column set was invalid, true otherwise
 */
bool XMLImport::validateColumns()
{
    colNames = db->listColumns();
    if (containsDuplicate(colNames, "cname")) {
        return false;
    }
    colCount = columnMap.count();
    for (int i = 0; i < colCount; i++) {
        if (!columnMap.contains(i)) {
            error = tr("Missing") + " cindex: " + QString::number(i);
            return false;
        }
        idList.append(columnMap[i]);
    }
    if (containsDuplicate(idList, "cid")) {
        return false;
    }
    db->updateDataFormat();
    return true;
}

/**
 * Make sure that all the sets of view columns are valid when taken as a
 * whole.  Returns false if there is a gap in a position index sequence, if
 * a column is used more than once in the same view, or if the all columns
 * view is missing or using a sequence that doesn't match the data columns
 * definition.
 *
 * @return True if all the view component columns are valid, false otherwise
 */
bool XMLImport::validateViewColumns()
{
    if (!indexMap.contains("_all")) {
        error = tr("Missing") + " vcview: _all";
        return false;
    }
    if (!validateIndexMap("vcindex", "vcname")) {
        return false;
    }
    return true;
}

/**
 * Make sure that all the sets of sorting columns are valid when taken as a
 * whole.  Returns false if there is a gap in a position index sequence or if
 * a column is used more than once in the same sorting.  Otherwise, adds all
 * the sortings defined in the XML file to the target database.
 *
 * @return True if all the sortings are valid, false otherwise
 */
bool XMLImport::validateSortColumns()
{
    if (!validateIndexMap("scindex", "scname")) {
        return false;
    }
    IndexMap::Iterator iter;
    QStringList allCols;
    QStringList descCols;
    for (iter = indexMap.begin(); iter != indexMap.end(); ++iter) {
        allCols.clear();
        descCols.clear();
        QString sname = iter.key();
        QMap<int,QString> mapping = iter.value();
        int count = mapping.count();
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
    return true;
}

/**
 * Make sure that all the sets of filter conditions are valid when taken as a
 * whole.  Returns false if there is a gap in a position index sequence or if
 * a condition is used more than once in the same filter.  Otherwise, adds
 * all the filters defined in the XML file to the target database.
 *
 * @return True if all the filters are valid, false otherwise
 */
bool XMLImport::validateFilterConditions()
{
    if (!validateIndexMap("fcposition", "fcposition")) {
        return false;
    }
    FilterMap::Iterator iter;
    for (iter = filterMap.begin(); iter != filterMap.end(); ++iter) {
        Filter *filter = iter.value();
        if (iter.key() != "_allrows") {
            // _allrows already added during database initialization
            db->addFilter(filter, false);
        }
        delete filter;
    }
    // validation of global table data must happen after everything is defined
    if (!validateGlobal()) {
        return false;
    }
    return true;
}

/**
 * Make sure that all the sets of calculation nodes are valid when taken as a
 * whole.  Returns false if a node ID is used more than once in the same
 * calculation, if a node's specified parent doesn't exist, or if one of the
 * calculations doesn't validate.
 *
 * @return True if all the calculations are valid, false otherwise
 */
bool XMLImport::validateCalcNodes()
{
    IndexMap::Iterator iter;
    IntList nodeIds;
    for (iter = indexMap.begin(); iter != indexMap.end(); ++iter) {
        QMap<int,QString> mapping = iter.value();
        QMap<int,QString>::Iterator iter2;
        nodeIds.clear();
        for (iter2 = mapping.begin(); iter2 != mapping.end(); ++iter2) {
            nodeIds.append(iter2.key());
        }
        if (containsDuplicate(nodeIds, "cnnodeid")) {
            return false;
        }
        for (iter2 = mapping.begin(); iter2 != mapping.end(); ++iter2) {
            QString parentId = iter2.value();
            if (!nodeIds.contains(parentId.toInt())) {
                error = tr("Invalid") + " cnparentid: " + parentId;
                return false;
            }
        }
        CalcNode *root = db->loadCalc(iter.key());
        error = isValidCalcNode(root);
        delete root;
        if (!error.isEmpty()) {
            return false;
        }
    }
    return true;
}

/**
 * Validate the provided calculation node.
 *
 * @param node The calculation node to be validated
 * @return A description of the error encountered, or an empty string if none
 */
QString XMLImport::isValidCalcNode(CalcNode *node)
{
    CalcNode::NodeType type = node->type();
    if (type < 0 || (type > CalcNode::TimeColumn && type < CALC_FIRST_OP)
        || type > CALC_LAST_OP) {
        return tr("Invalid") + " cntype: " + QString::number(type);
    }
    QString value = node->value();
    if (type == CalcNode::Constant) {
        bool ok;
        Formatting::parseDouble(value, &ok);
        if (!ok) {
            return tr("Invalid") + " cnvalue: " + value;
        }
    }
    else if (type == CalcNode::DateConstant) {
        QString error = db->isValidValue(DATE, value);
        if (!error.isEmpty()) {
            return tr("Invalid") + " cnvalue: " + value;
        }
    }
    else if (type == CalcNode::Column) {
        if (!colNames.contains(value)) {
            return tr("Invalid") + " cnvalue: " + value;
        }
        int colType = db->getType(value);
        if (colType != INTEGER && colType != FLOAT && colType != BOOLEAN) {
            return tr("Invalid") + " cnvalue: " + value;
        }
    }
    else if (type == CalcNode::DateColumn) {
        if (!colNames.contains(value)) {
            return tr("Invalid") + " cnvalue: " + value;
        }
        if (db->getType(value) != DATE) {
            return tr("Invalid") + " cnvalue: " + value;
        }
    }
    CalcNodeList children = node->getChildren();
    int count = children.count();
    int max = node->maxChildren();
    if (max != -1 && count > max) {
        return tr("Too many child nodes for node type") + ": "
               + QString::number(type);
    }
    for (int i = 0; i < count; i++) {
        QString error = isValidCalcNode(children[i]);
        if (!error.isEmpty()) {
            return error;
        }
    }
    return "";
}

/**
 * Determine if a provided string is a valid default value for a column of
 * the specified type.  This is different than determining if a string is a
 * valid data value of that type because dates and times have special rules
 * for default values.
 *
 * @param cname The name of the column whose default is being checked
 * @param ctype The data type of the column
 * @param cdefault The default value specified in the XML file
 * @return True if the default value is valid, false otherwise
 */
bool XMLImport::isValidDefault(const QString &cname, int ctype,
                               const QString &cdefault)
{
    // in case it's needed...
    QString errorMsg = tr("Invalid") + " cdefault: " + cdefault
                       + " (" + cname + ")";
    if (ctype == DATE) {
        bool ok = false;
        int value = cdefault.toInt(&ok);
        if (!ok || (value != 0 && value != 17520914)) {
            error = errorMsg;
            return false;
        }
    }
    else if (ctype == TIME) {
        bool ok = false;
        int value = cdefault.toInt(&ok);
        if (!ok || (value != 0 && value != -1)) {
            error = errorMsg;
            return false;
        }
    }
    else {
        if (!db->isValidValue(ctype, cdefault).isEmpty()) {
            error = errorMsg;
            return false;
        }
    }
    return true;
}

/**
 * Determine if the given condition operator code and case sensitivity are
 * valid for the specified data type.
 *
 * @param type The data type of the column being compared
 * @param op The operator code specified in the XML file
 * @param cs Case sensitivity indicator
 * @return True if the provided values are valid, false otherwise
 */
bool XMLImport::isValidOperator(int type, int op, int cs)
{
    // in case it's needed...
    QString opError = tr("Invalid") + " fcoperator: " + QString::number(op)
                      + " (ctype = " + QString::number(type) + ")";
    if (type == BOOLEAN) {
        if (op != Condition::Equals) {
            error = opError;
            return false;
        }
    }
    else if (type >= FIRST_ENUM) {
        if (op != Condition::Equals && op != Condition::NotEqual) {
            error = opError;
            return false;
        }
        if (cs == 0) {
            error = tr("Invalid") + " fccase: 0 (ctype = "
                    + QString::number(type) + ")";
            return false;
        }
    }
    else if (type == STRING || type == NOTE) {
        if (op != Condition::Equals && op != Condition::Contains 
                && op != Condition::StartsWith && op != Condition::NotEqual) {
            error = opError;
            return false;
        }
    }
    else {
        if (op != Condition::Equals && op != Condition::NotEqual
                && op != Condition::LessThan && op != Condition::LessEqual
                && op != Condition::GreaterThan
                && op != Condition::GreaterEqual) {
            error = opError;
            return false;
        }
    }
    return true;
}

/**
 * Determine if the given list (of names for the given element type) contains
 * any duplicate entries.
 *
 * @param names The list of all names for a certain kind of database item
 * @param element The name of the element for which the check is being done
 * @return True if at least one duplicate was found, false otherwise
 */
bool XMLImport::containsDuplicate(const QStringList &names,
                                  const QString &element)
{
    QStringList copy(names);
    int count = copy.count();
    for (int i = 0; i < count; i++) {
        QString name = copy[0];
        copy.removeAt(0);
        if (copy.contains(name)) {
            error = tr("Duplicate") + " " + element + ": " + name;
            return true;
        }
    }
    return false;
}

/**
 * Determine if the given list (of IDs for the given element type) contains
 * any duplicate entries.
 *
 * @param items The list of all IDs for a certain kind of database item
 * @param element The name of the element for which the check is being done
 * @return True if at least one duplicate was found, false otherwise
 */
bool XMLImport::containsDuplicate(const IntList &items, const QString &element)
{
    IntList copy(items);
    int count = copy.count();
    for (int i = 0; i < count; i++) {
        int item = copy[0];
        copy.removeAt(0);
        if (copy.contains(item)) {
            error = tr("Duplicate") + " " + element + ": "
                    + QString::number(item);
            return true;
        }
    }
    return false;
}

/**
 * Validate the data currently in the <code>indexMap</code> attribute.  Looks
 * for gaps in the position index sequence and duplicate names.  When
 * validating views, also makes sure that the all columns view uses the same
 * sequence as the database format definition.
 *
 * @param indexElement The name of the element containing the index property
 * @param stringElement The name of the element containing the name property
 * @return False if a problem was found with the data, true otherwise
 */
bool XMLImport::validateIndexMap(const QString &indexElement,
                                 const QString &stringElement)
{
    IndexMap::Iterator iter;
    int i;
    QStringList itemList;
    for (iter = indexMap.begin(); iter != indexMap.end(); ++iter) {
        QMap<int,QString> mapping = iter.value();
        int indexCount = mapping.count();
        itemList.clear();
        for (i = 0; i < indexCount; i++) {
            if (!mapping.contains(i)) {
                error = tr("Missing") + " " + indexElement + ": "
                        + QString::number(i);
                return false;
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
                error = tr("Incorrect _all view column sequence");
                return false;
            }
        }
        if (containsDuplicate(itemList, stringElement)) {
            return false;
        }
    }
    return true;
}

/**
 * Set the most recently parsed text data to be the value of the named
 * property.  Performs validation on the value based on the declared type of
 * that property (boolean, integer, non-negative integer, etc.), and makes
 * sure that it isn't a duplicate element.  Called for every closing tag,
 * although the value isn't always needed.
 *
 * @param name The name of the element the text was inside of
 * @return True if the text is a non-duplicate valid value of its type
 */
bool XMLImport::setField(const QString &name)
{
    if (!containers.contains(parent)) {
        // no meaningful text content, move on
        return true;
    }
    bool ok = false;
    if (booleans.contains(name)) {
        int value = text.toInt(&ok);
        if (!ok || value < 0 || value > 1) {
            error = name + ": " + tr("must be 0 or 1");
            return false;
        }
    }
    else if (integers.contains(name)) {
        text.toInt(&ok);
        if (!ok) {
            error = name + ": " + tr("must be an integer");
            return false;
        }
    }
    else if (nonNegativeIntegers.contains(name)) {
        int value = text.toInt(&ok);
        if (!ok || value < 0) {
            error = name + ": " + tr("must be a non-negative integer");
            return false;
        }
    }
    else if (positiveIntegers.contains(name)) {
        int value = text.toInt(&ok);
        if (!ok || value <= 0) {
            error = name + ": " + tr("must be a positive integer");
            return false;
        }
    }
    if (parent != "r" && fields.contains(name)) {
        error = tr("Duplicate element") + ": " + name;
        return false;
    }
    fields.insert(name, text);
    return true;
}

/**
 * Get the text content of the specified required element.  Sets an
 * appropriate error message if no such content was found.
 *
 * @param name The name of the element whose content is desired
 * @return The value of the named element, or an empty string if absent
 */
QString XMLImport::getField(const QString &name)
{
    if (!fields.contains(name)) {
        error = tr("Missing element") + ": " + name;
        return "";
    }
    return fields[name];
}

/**
 * Get the text content of the specified optional element.  Uses the provided
 * default value if no such content was found.
 *
 * @param name The name of the element whose content is desired
 * @param defaultVal The default value
 * @return The value of the named element, or the default value if absent
 */
QString XMLImport::getOptionalField(const QString &name,
                                    const QString &defaultVal)
{
    if (!fields.contains(name)) {
        return defaultVal;
    }
    return fields[name];
}

/**
 * Gets the last parsed value for data column with the specified ID.  Sets an
 * appropriate error message if no value was found for that column.
 *
 * @param columnId The ID of the column whose value is desired
 * @return The latest value in that column, or an empty string if absent
 */
QString XMLImport::getDataField(int columnId)
{
    QString idString = QString::number(columnId);
    if (!fields.contains(idString)) {
        error = tr("Missing data for column ID %1").arg(idString);
        return "";
    }
    return fields[idString];
}

/**
 * Build a mapping between allowed element names and their parent elements.
 * Used for validation purposes.
 */
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
    parents.insert("calcs", "portabase");
    parents.insert("calcnodes", "portabase");
    parents.insert("data", "portabase");

    parents.insert("gversion", "global");
    parents.insert("gview", "global");
    parents.insert("gsort", "global");
    parents.insert("gfilter", "global");
    // accidentally exported in 1.6, allow it and ignore it
    parents.insert("gcrypt", "global");

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
    parents.insert("vsort", "view");
    parents.insert("vfilter", "view");

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

    parents.insert("calc", "calcs");
    parents.insert("calcid", "calc");
    parents.insert("calcdecimals", "calc");

    parents.insert("calcnode", "calcnodes");
    parents.insert("cnid", "calcnode");
    parents.insert("cnnodeid", "calcnode");
    parents.insert("cnparentid", "calcnode");
    parents.insert("cntype", "calcnode");
    parents.insert("cnvalue", "calcnode");

    parents.insert("r", "data");
    parents.insert("s", "r");
    parents.insert("i", "r");
    parents.insert("f", "r");
    parents.insert("b", "r");
    parents.insert("n", "r");
    parents.insert("d", "r");
    parents.insert("t", "r");
    parents.insert("c", "r");
    parents.insert("q", "r");
    parents.insert("p", "r");
    parents.insert("e", "r");
}
