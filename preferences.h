/*
 * preferences.h
 *
 * (c) 2002 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <qdialog.h>
#include <qfontdatabase.h>

class QCheckBox;
class QComboBox;
class QFont;
class QLabel;
class QVBox;

class Preferences: public QDialog
{
    Q_OBJECT
public:
    Preferences(QWidget *parent = 0, const char *name = 0, WFlags f = 0);
    ~Preferences();

    QFont applyChanges();

protected:
    void resizeEvent(QResizeEvent *event);

private slots:
    void updateSizes(int selected);
    void updateSample(int selectedSize);

private:
    QVBox *vbox;
    QFontDatabase fontdb;
    QStringList fonts;
    QValueList<int> sizes;
    QComboBox *fontName;
    QComboBox *fontSize;
    QLabel *sample;
    QCheckBox *confirmDeletions;
    QCheckBox *booleanToggle;
    QCheckBox *showSeconds;
};

#endif
