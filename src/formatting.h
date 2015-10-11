/*
 * formatting.h
 *
 * (c) 2010-2011,2015 by Jeremy Bowman <jmbowman@alum.mit.edu>
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

#include <QDate>
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

    static bool isNoneDate(const QDate &date);
    static QString dateToString(int date);
    static QString dateToString(const QDate &date);
    static QString timeToString(int time);
    static QString dateTimeToString(const QDateTime &dateTime);
    static QString parseTimeString(const QString &value, bool *ok);

private:
    static QLocale cLocale; /**< The "C" locale used internally */
    static QLocale systemLocale; /**< The user's selected locale */
    static QChar localDecimalPoint; /**< Decimal point character in the user's locale */
    static QChar localExponential; /**< Exponential character in the user's locale */
    static QChar localPercent; /**< Percentage character in the user's locale */
    static QRegExp cRegExp; /**< Decimal number pattern in the "C" locale */
    static QRegExp localRegExp; /**< Decimal number pattern in the user's locale */
    static QString dateFormat; /**< The short format to use for displaying date values */
    static QString timeFormat; /**< The format to use for displaying time values */
};

#endif // FORMATTING_H
