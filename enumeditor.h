/*
 * enumeditor.h
 *
 * (c) 2002-2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef ENUMEDITOR_H
#define ENUMEDITOR_H

#include <mk4.h>
#include <qdialog.h>

#define ADD_OPTION 0
#define RENAME_OPTION 1
#define DELETE_OPTION 2

#define NOT_SORTED 0
#define ASCENDING 1
#define DESCENDING 2

class Database;
class QLineEdit;
class QListBox;

class EnumEditor: public QDialog
{
    Q_OBJECT
public:
    EnumEditor(QWidget *parent = 0, const char *name = 0, WFlags f = 0);
    ~EnumEditor();

    int edit(Database *subject, QString enumName);
    void applyChanges();
    QString getName();

private:
    void updateList();
    bool hasValidName();
    bool isValidOption(QString option);
    QStringList listCurrentOptions();

private slots:
    void sortOptions();
    void importOptions();
    void exportOptions();
    void addOption();
    void editOption();
    void deleteOption();
    void moveUp();
    void moveDown();

private:
    QLineEdit *nameBox;
    QListBox *listBox;
    Database *db;
    QString originalName;
    c4_View info;
    c4_StringProp eeiName;
    c4_IntProp eeiIndex;
    c4_View changes;
    c4_IntProp eecIndex;
    c4_IntProp eecType;
    c4_StringProp eecOldName;
    c4_StringProp eecNewName;
    int sorting;
};

#endif
