/*
 * columneditor.h
 *
 * (c) 2002 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef COLUMNEDITOR_H
#define COLUMNEDITOR_H

#include <qdialog.h>

class CalcNode;
class DBEditor;
class Database;
class NoteButton;
class NumberWidget;
class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QWidgetStack;

class ColumnEditor: public QDialog
{
    Q_OBJECT
public:
    ColumnEditor(Database *dbase, DBEditor *parent = 0, const char *name = 0,
                 WFlags f = 0);
    ~ColumnEditor();

    QString name();
    void setName(QString newName);
    int type();
    void setType(int newType);
    QString defaultValue();
    void setDefaultValue(QString newDefault);
    CalcNode *calculation(int *decimals);
    void setCalculation(CalcNode *root, int decimals);
    void setTypeEditable(bool flag);
    int exec();

private slots:
    void updateDefaultWidget(int newType);
    void editCalculation();

private:
    Database *db;
    DBEditor *dbEditor;
    QLineEdit *nameBox;
    QComboBox *typeBox;
    QLabel *defaultLabel;
    QWidgetStack *defaultStack;
    QCheckBox *defaultCheck;
    QLineEdit *defaultLine;
    NoteButton *defaultNote;
    NumberWidget *defaultInteger;
    NumberWidget *defaultFloat;
    QComboBox *defaultDate;
    QComboBox *defaultTime;
    QComboBox *defaultEnum;
    QPushButton *calcButton;
    NumberWidget *defaultSequence;
    CalcNode *calcRoot;
    int calcDecimals;
    int lastType;
};

#endif
