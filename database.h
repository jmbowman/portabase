/*
 * database.h
 *
 * (c) 2002-2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef DATABASE_H
#define DATABASE_H

#include <mk4.h>
#include <qpixmap.h>
#include "datatypes.h"

#if defined(Q_WS_QWS)
#include <qpe/timestring.h>
#else
#include "desktop/timestring.h"
#endif

#define FILE_VERSION 11

#define OPEN_NEWER_VERSION 0
#define OPEN_SUCCESS 1
#define OPEN_ENCRYPTED 2

class CalcNode;
class Condition;
class Crypto;
class Filter;
class ImageUtils;
class QDate;
class QString;
class QStringList;
class View;

class Database
{
public:
    Database(QString path, int *result, int encrypt=0);
    ~Database();

    QString load();
    bool encrypted();
    QString setPassword(const QString &pass, bool newPass);
    QString changePassword(const QString &oldPass, const QString &newPass);
    void updateDateTimePrefs();
    QString currentView();
    View *getView(const QString &name, bool applyDefaults=FALSE,
                  bool setAsCurrent=TRUE);
    QString getDefaultSort(const QString &viewName);
    QString getDefaultFilter(const QString &viewName);
    QStringList listViews();
    void addView(const QString &name, const QStringList &names,
                 const QString &defaultSort, const QString &defaultFilter,
                 int rpp=-1, int deskrpp=-1);
    void deleteView(const QString &name);
    void setViewColWidths(int *widths);
    void setViewRowsPerPage(int rpp);
    void setViewDefaults(const QString &sorting, const QString &filter);

    QStringList listColumns();
    int getIndex(const QString &column);
    void setIndex(const QString &column, int index);
    int getType(const QString &column);
    IntList listTypes();
    QString getDefault(const QString &column);
    void setDefault(const QString &column, const QString &value);
    QString getColId(const QString &column, int type=-1);
    QString isValidValue(int type, const QString &value);
    void addColumn(int index, const QString &name, int type,
                   const QString &defaultVal, int id=-1);
    void renameColumn(const QString &oldName, const QString &newName);
    void deleteColumn(const QString &name);
    void renameView(const QString &oldName, const QString &newName);
    QStringList listViewColumns(const QString &viewName);
    void addViewColumn(const QString &viewName, const QString &columnName,
                       int index=-1, int width=-1, int deskwidth=-1);
    void deleteViewColumn(const QString &viewName, const QString &columnName);
    void setViewColumnSequence(const QString &viewName,
                               const QStringList &colNames);
    void updateDataFormat();
    QStringList getRow(int rowId, ImageUtils *utils=0);

    QString currentSorting();
    QStringList listSortings();
    bool getSortingInfo(const QString &sortingName, QStringList *allCols,
                        QStringList *descCols);
    void addSorting(const QString &name, const QStringList &allCols,
                    const QStringList &descCols);
    void deleteSorting(const QString &name);
    void deleteSortingColumn(const QString &sortName, const QString &columnName);
    c4_View getData();
    c4_View sortData(c4_View filteredData, const QString &column, bool ascending);
    c4_View sortData(c4_View filteredData, const QString &sortingName);

    QString currentFilter();
    QStringList listFilters();
    Filter *getFilter(const QString &name);
    void addFilter(Filter *filter, bool setAsCurrent=TRUE);
    void deleteFilter(const QString &name);
    void deleteFilterColumn(const QString &filterName, const QString &columnName);
    int getConditionCount(const QString &filterName);
    Condition *getCondition(const QString &filterName, int index);

    QStringList listEnums();
    QStringList listEnumOptions(int id);
    void addEnum(const QString &name, const QStringList &options,
                 int index=-1, int id=-1);
    void renameEnum(const QString &oldName, const QString &newName);
    void deleteEnum(const QString &name);
    int getEnumId(const QString &name);
    QString getEnumName(int id);
    QStringList columnsUsingEnum(const QString &enumName);
    void setEnumSequence(const QStringList &names);
    void addEnumOption(const QString &enumName, const QString &option,
                       int index=-1);
    void renameEnumOption(const QString &enumName, const QString &oldName,
                          const QString &newName);
    void deleteEnumOption(const QString &enumName, const QString &option,
                          const QString &replace);
    void setEnumOptionSequence(const QString &enumName,
                               const QStringList &options);

    CalcNode *loadCalc(const QString &colName, int *decimals=0);
    void updateCalc(const QString &colName, CalcNode *root, int decimals);
    QString formatDouble(double value, int decimals=2);
    void calculateAll();

    QString addRow(QStringList values, int *rowId=0,
                   bool acceptSequenceVals=FALSE, bool fromcsv=FALSE);
    void updateRow(int rowId, const QStringList &values);
    void deleteRow(int id);
    void toggleBoolean(int rowId, const QString &colName);
    char *getBinaryField(int rowId, const QString &colName, int *size);
    void setBinaryField(int rowId, const QString &colName, char *dataArray,
                        int size);
    void commit();
    bool isNoneDate(const QDate &date);
    QString dateToString(int date);
    QString dateToString(const QDate &date);
    QString timeToString(int time);
    QString parseTimeString(QString value, bool *ok);
    void setShowSeconds(bool show);
    void setImportBasePath(const QString &path);
    QStringList importFromCSV(const QString &filename,
                              const QString &encoding);
    void exportToXML(QString filename, c4_View &fullView,
                     c4_View &filteredView, QStringList cols);
    void setGlobalInfo(const QString &view, const QString &sorting,
                       const QString &filter);
    int addCalcNode(int calcId, CalcNode *root, int nodeId, int parentId);

private:
    c4_View createEmptyView(const QStringList &colNames);
    void replaceEnumOption(int enumId, const QString &oldOption,
                           const QString &newOption);
    QString makeColId(int id, int type);
    QString formatString(bool old = FALSE);
    void updateDataColumnFormat();
    void updateEncoding();
    void updateEncoding(c4_View &view);
    void fixConditionIndices();
    void addDesktopStats();
    void addEnumDataIndices();
    void updateEnumDataIndices(const QString &enumName);
    void addViewDefaults();
    CalcNode *loadCalc(int colId, int *decimals);
    void deleteCalc(int colId);
    void deleteCalcColumn(const QString &columnName);
    void calculateAll(int colId, CalcNode *root, int decimals);

private:
#if defined(Q_WS_QWS)
    DateFormat::Order dateOrder;
#else
    PBDateFormat::Order dateOrder;
#endif
    QString dateSeparator;
    bool ampm;
    bool showSeconds;
    // the data file
    c4_Storage *file;
    // also the data file if unencrypted, wrapped data otherwise
    c4_Storage *storage;
    Crypto *crypto;
    int version;
    bool newFile;
    QString importBasePath;
    c4_View columns;
    c4_View views;
    c4_View viewColumns;
    c4_View sorts;
    c4_View sortColumns;
    c4_View filters;
    c4_View filterConditions;
    c4_View enums;
    c4_View enumOptions;
    c4_View calcs;
    c4_View calcNodes;
    c4_View global;
    c4_View data;
    View *curView;
    Filter *curFilter;
    int maxId;
    c4_IntProp Id;
    // "_columns" view
    c4_IntProp cIndex;
    c4_StringProp cName;
    c4_IntProp cType;
    c4_StringProp cDefault;
    c4_IntProp cId;
    // "_views" view
    c4_StringProp vName;
    c4_IntProp vRpp;
    c4_IntProp vDeskRpp;
    c4_StringProp vSort;
    c4_StringProp vFilter;
    // "_viewcolumns" view
    c4_StringProp vcView;
    c4_IntProp vcIndex;
    c4_StringProp vcName;
    c4_IntProp vcWidth;
    c4_IntProp vcDeskWidth;
    // "_sorts" view
    c4_StringProp sName;
    // "_sortcolumns" view
    c4_StringProp scSort;
    c4_IntProp scIndex;
    c4_StringProp scName;
    c4_IntProp scDesc;
    // "_filters" view
    c4_StringProp fName;
    // "_filterconditions" view
    c4_StringProp fcFilter;
    c4_IntProp fcPosition;
    c4_StringProp fcColumn;
    c4_IntProp fcOperator;
    c4_StringProp fcConstant;
    c4_IntProp fcCase;
    // "_enums" view
    c4_StringProp eName;
    c4_IntProp eId;
    c4_IntProp eIndex;
    // "_enumoptions" view
    c4_IntProp eoEnum;
    c4_IntProp eoIndex;
    c4_StringProp eoText;
    // "_calcs" view
    c4_IntProp calcId;
    c4_IntProp calcDecimals;
    // "_calcnodes" view
    c4_IntProp cnId;
    c4_IntProp cnNodeId;
    c4_IntProp cnParentId;
    c4_IntProp cnType;
    c4_StringProp cnValue;
    // "_global" view
    c4_IntProp gVersion;
    c4_StringProp gView;
    c4_StringProp gSort;
    c4_StringProp gFilter;
    c4_IntProp gCrypt;
};

#endif
