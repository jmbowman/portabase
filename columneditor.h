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

class QCheckBox;
class QComboBox;
class QLineEdit;
class NoteButton;
class QWidgetStack;

class ColumnEditor: public QDialog
{
    Q_OBJECT
public:
    ColumnEditor(QWidget *parent = 0, const char *name = 0, WFlags f = 0);
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
    QLineEdit *nameBox;
    QComboBox *typeBox;
    QWidgetStack *defaultStack;
    QCheckBox *defaultCheck;
    QLineEdit *defaultLine;
    NoteButton *defaultNote;
};

#endif
