/*
 * preferences.h
 *
 * (c) 2002-2004,2009-2011 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file preferences.h
 * Header file for Preferences
 */

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QFontDatabase>
#include <QPixmap>
#include "datatypes.h"
#include "pbdialog.h"

class QtColorPicker;
class QCheckBox;
class QComboBox;
class QFont;
class QLabel;
class QQMenuHelper;
class QSettings;
class QSpinBox;
class QTabWidget;

/**
 * Application preferences dialog.  Contains settings for time and date
 * formatting, font name and size, text wrapping in notes, data grid paging
 * options, etc.
 */
class Preferences: public PBDialog
{
    Q_OBJECT
public:
    Preferences(QQMenuHelper *menuHelper, QWidget *parent = 0);

    QFont applyChanges();

private slots:
    void updateSizes(int selected);
    void updateSample(int selectedSize);

private:
    void addGeneralTab(QSettings *settings);
    void addDateTimeTab(QSettings *settings);
    void addAppearanceTab(QSettings *settings);
    void configureColorPicker(QtColorPicker *picker);

private:
    QQMenuHelper *mh; /**< Support code for any document-based application */
    QTabWidget *tabs; /**< Tab widget used on all non-Fremantle platforms */
    QWidget *panel; /**< Large scrollable preferences panel used on Fremantle */
    QCheckBox *variableHeightRows; /**< Option to allow row heights to vary on Fremantle */
    QFontDatabase fontdb; /**< System font database */
    IntList sizes; /**< List of displayed font sizes */
    QComboBox *fontName; /**< Font name selection list */
    QComboBox *fontSize; /**< Font size selection list */
    QLabel *sample; /**< Sample of text using the selected font */
    QCheckBox *confirmDeletions; /**< Option to display confirmation dialogs for item deletions */
    QCheckBox *booleanToggle; /**< Option to toggle boolean fields by clicking on them in the data viewer */
    QCheckBox *showSeconds; /**< Option to show seconds for time fields */
    QCheckBox *noteWrap; /**< Option to autowrap text in the node editor */
    QComboBox *wrapType; /**< Option to force note text wrapping at line end rather than whitespace */
    QCheckBox *pagedDisplay; /**< Option to pages of data rows, rather than all in a scrolling list */
    QCheckBox *singleClickShow; /**< Option to show the row viewer with a single click in the data grid */
    QCheckBox *autoRotate; /**< Option to auto-rotate with Fremantle device orientation */
    QSpinBox *rowsPerPage; /**< Number of rows to display on each page of the data view */
    QCheckBox *smallScreen; /**< Option to use settings for PDA/phone-sized screens */
    QCheckBox *useAlternating; /**< Option to turn off alternating row colors on Fremantle*/
    QtColorPicker *evenButton; /**< Button to select the color of even rows */
    QtColorPicker *oddButton; /**< Button to select the color of odd rows */
    QStringList dateFormats; /**< List of standard date formatting styles */
    QComboBox *dateFormatCombo; /**< Date format selection list */
    QComboBox *ampmCombo; /**< Option to show AM/PM times rather than 24-hour times */
    QComboBox *weekStartCombo; /**< First day of the week selection list */
};

#endif
