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

class Database;
class NoteButton;
class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QWidgetStack;

class ColumnEditor: public QDialog
{
    Q_OBJECT
public:
    ColumnEditor(Database *dbase, QWidget *parent = 0, const char *name = 0,
                 WFlags f = 0);
    ~ColumnEditor();

    QString name();
    void setName(QString newName);
    int type();
    void setType(int newType);
    QString defaultValue();
    void setDefaultValue(QString newDefault);
    void setTypeEditable(bool flag);

private slots:
    void updateDefaultWidget(int newType);

private:
    Database *db;
    QLineEdit *nameBox;
    QComboBox *typeBox;
    QWidgetStack *defaultStack;
    QCheckBox *defaultCheck;
    QLineEdit *defaultLine;
    NoteButton *defaultNote;
    QComboBox *defaultDate;
    QComboBox *defaultEnum;
    int lastType;
};

#endif
