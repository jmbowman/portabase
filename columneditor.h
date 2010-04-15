/*
 * columneditor.h
 *
 * (c) 2002-2004,2008-2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file columneditor.h
 * Header file for ColumnEditor
 */

#ifndef COLUMNEDITOR_H
#define COLUMNEDITOR_H

#include "qqutil/qqdialog.h"

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
class QStackedWidget;

/**
 * A dialog for setting the properties of a database column.  Used by
 * DBEditor.
 */
class ColumnEditor: public QQDialog
{
    Q_OBJECT
public:
    ColumnEditor(Database *dbase, DBEditor *parent = 0);

    QString name();
    void setName(const QString &newName);
    int type();
    void setType(int newType);
    QString defaultValue();
    void setDefaultValue(const QString &newDefault);
    CalcNode *calculation(int *decimals);
    void setCalculation(CalcNode *root, int decimals);
    void setTypeEditable(bool flag);

public slots:
    int exec();

private slots:
    void updateDefaultWidget(int newType);
    void editCalculation();

private:
    Database *db; /**< The database in use */
    DBEditor *dbEditor; /**< The database format editor dialog */
    QLineEdit *nameBox; /**< Column name entry field */
    QComboBox *typeBox; /**< Column type selection list */
    QLabel *defaultLabel; /**< Default value UI label (sometimes vanishes) */
    QStackedWidget *defaultStack; /**< Stack of default value entry widgets */
    QCheckBox *defaultCheck; /**< Default value checkbox for booleans */
    QLineEdit *defaultLine; /**< Default value field for strings */
    NoteButton *defaultNote; /**< Default value field for notes */
    NumberWidget *defaultInteger; /**< Default value field for integers */
    NumberWidget *defaultFloat; /**< Default value field for floats */
    QComboBox *defaultDate; /**< Default value list for dates */
    QComboBox *defaultTime; /**< Default value list for times */
    QComboBox *defaultEnum; /**< Default value list for enums */
    QPushButton *calcButton; /**< Button for defining calculated columns */
    NumberWidget *defaultSequence; /**< Next value field for sequences */
    QWidget *defaultBlank; /**< Placeholder shown for images (no default) */
    CalcNode *calcRoot; /**< Root node of a calculated column definition */
    int calcDecimals; /**< Number of decimal places to show for a calculated column */
    int lastType; /**< The last selected column type */
};

#endif
