/*
 * formatting.h
 *
 * (c) 2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file formatting.h
 * Header file for Formatting
 */

#ifndef FORMATTING_H
#define FORMATTING_H

#include <QLocale>
#include <QRegExp>

class QSettings;

/**
 * Collection of utility methods for formatting data types for display (or
 * parsing them from entered strings).  The behavior of some of these is
 * dependent on preference settings.
 */
class Formatting
{
public:
    static void updatePreferences(QSettings *settings);
    static QString fromLocalDouble(const QString &value);
    static QString toLocalDouble(const QString &value);
    static double parseDouble(const QString &value, bool *ok=0);
    static QString formatDouble(double value, int decimals=-1);
    static QString toLocalInt(const QString &value);

private:
    static QLocale cLocale;
    static QLocale systemLocale;
    static QChar localDecimalPoint;
    static QChar localExponential;
    static QChar localPercent;
    static QRegExp cRegExp;
    static QRegExp localRegExp;
};

#endif // FORMATTING_H
