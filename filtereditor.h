/*
 * filtereditor.h
 *
 * (c) 2002 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FILTEREDITOR_H
#define FILTEREDITOR_H

#include <qdialog.h>

class ConditionEditor;
class Database;
class Filter;
class QLineEdit;
class QListBox;
class QVBox;

class FilterEditor: public QDialog
{
    Q_OBJECT
public:
    FilterEditor(QWidget *parent = 0, const char *name = 0, WFlags f = 0);
    ~FilterEditor();

    int edit(Database *subject, QString filterName);
    void applyChanges();
    QString getName();

protected:
    void resizeEvent(QResizeEvent *event);

private:
    void updateList();
    bool hasValidName();

private slots:
    void addCondition();
    void editCondition();
    void deleteCondition();
    void moveUp();
    void moveDown();

private:
    QVBox *vbox;
    QLineEdit *nameBox;
    QListBox *listBox;
    Database *db;
    QString originalName;
    Filter *filter;
    ConditionEditor *conditionEditor;
};

#endif
