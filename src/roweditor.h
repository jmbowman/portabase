/*
 * roweditor.h
 *
 * (c) 2002-2004,2008-2010,2015 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file roweditor.h
 * Header file for RowEditor
 */

#ifndef ROWEDITOR_H
#define ROWEDITOR_H

#include <QStringList>
#include <QList>
#include "datatypes.h"
#include "pbdialog.h"

class CalcWidget;
class Database;
class DateWidget;
class DynamicEdit;
class QCheckBox;
class QComboBox;
class QLabel;
class ImageSelector;
class NoteButton;
class NumberWidget;
class TimeWidget;

typedef QList<QCheckBox*> CheckBoxList;
typedef QList<NoteButton*> NoteButtonList;
typedef QList<DateWidget*> DateWidgetList;
typedef QList<TimeWidget*> TimeWidgetList;
typedef QList<NumberWidget*> NumberWidgetList;
typedef QList<QComboBox*> ComboBoxList;
typedef QList<DynamicEdit*> DynamicEditList;
typedef QList<CalcWidget*> CalcWidgetList;
typedef QList<QLabel*> LabelList;
typedef QList<ImageSelector*> ImageSelectorList;

/**
 * Dialog for editing the values in one row of the database.
 */
class RowEditor: public PBDialog
{
    Q_OBJECT
public:
    explicit RowEditor(QWidget *parent = 0);

    int edit(Database *subject, int rowId=-1, bool copy=false);
    bool isValid();
    QStringList getRow(bool doCalcs=true);

protected:
    void showEvent(QShowEvent *event);

private:
    void addContent(int rowId);

private:
    Database *db; /**< The database being edited */
    QStringList colNames; /**< Ordered list of all the database's column names */
    IntList colTypes; /**< Ordered list of all the database's column types */
    CheckBoxList checkBoxes; /**< List of this dialog's boolean edit widgets */
    NoteButtonList noteButtons; /**< List of this dialog's note edit widgets */
    DateWidgetList dateWidgets; /**< List of this dialog's date edit widgets */
    TimeWidgetList timeWidgets; /**< List of this dialog's time edit widgets */
    NumberWidgetList numberWidgets; /**< List of this dialog's number edit widgets */
    ComboBoxList comboBoxes; /**< List of this dialog's enum edit widgets */
    DynamicEditList dynamicEdits; /**< List of this dialog's string edit widgets */
    CalcWidgetList calcWidgets; /**< List of this dialog's calculation display widgets */
    LabelList sequenceLabels; /**< List of this dialog's sequence number display widgets */
    ImageSelectorList imageSelectors; /**< List of this dialog's image selection widgets */
    QWidget *initialFocus; /**< The widget which is to receive focus when the dialog is shown */
};

#endif
