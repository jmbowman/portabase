/*
 * preferences.h
 *
 * (c) 2002-2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef PREFERENCES_H
#define PREFERENCES_H

#if defined(DESKTOP)
#include "desktop/timestring.h"
#endif

#include <qfontdatabase.h>
#include <qpixmap.h>
#include "pbdialog.h"

class ColorButton;
class QCheckBox;
class QComboBox;
class QFont;
class QLabel;
class QListView;
class QTabWidget;

class Preferences: public PBDialog
{
    Q_OBJECT
public:
    Preferences(QWidget *parent = 0, const char *name = 0, WFlags f = 0);
    ~Preferences();

    QFont applyChanges();
    static void menuConfiguration(QStringList &top, QStringList &file);
    static void buttonConfiguration(QStringList &shown, QStringList &hidden);

private slots:
    void updateSizes(int selected);
    void updateSample(int selectedSize);
    void menuUp();
    void menuDown();
    void buttonUp();
    void buttonDown();

private:
    void addOptionsTab(QTabWidget *tabs);
    void addAppearanceTab(QTabWidget *tabs);
    void addMenusTab(QTabWidget *tabs);
    void addButtonsTab(QTabWidget *tabs);
    void moveSelectionUp(QListView *table);
    void moveSelectionDown(QListView *table);

private:
    QFontDatabase fontdb;
    QStringList fonts;
    QValueList<int> sizes;
    int sizeFactor;
    QComboBox *fontName;
    QComboBox *fontSize;
    QLabel *sample;
    QCheckBox *confirmDeletions;
    QCheckBox *booleanToggle;
    QCheckBox *showSeconds;
    QCheckBox *noteWrap;
    QComboBox *wrapType;
    QCheckBox *pagedDisplay;
    ColorButton *evenButton;
    ColorButton *oddButton;
#if defined(DESKTOP)
    PBDateFormat date_formats[4];
    QComboBox *dateFormatCombo;
    QComboBox *ampmCombo;
    QComboBox *weekStartCombo;
#endif
    QStringList menuList;
    QStringList menuLabelList;
    QListView *menuTable;
    QStringList buttonList;
    QStringList buttonLabelList;
    QStringList buttonResourceList;
    QListView *buttonTable;
};

#endif
