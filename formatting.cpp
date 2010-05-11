/*
 * formatting.cpp
 *
 * (c) 2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file formatting.cpp
 * Source file for Formatting
 */

#include <QSettings>
#include "formatting.h"

QLocale Formatting::cLocale = QLocale::c();
QLocale Formatting::systemLocale = QLocale::system();
QChar Formatting::localDecimalPoint = Formatting::systemLocale.decimalPoint();
QChar Formatting::localExponential = Formatting::systemLocale.exponential();
QChar Formatting::localPercent = Formatting::systemLocale.percent();
QRegExp Formatting::cRegExp("([^E%\\.]*)(\\.[0-9]*)?(E[0-9]+)?(\\s*%)?");
QRegExp Formatting::localRegExp(QString("([^%1%2%3]*)(%1[0-9]*)?(%2[0-9]+)?(\\s*%3)?")
                                .arg((Formatting::localDecimalPoint == '.') ? QString("\\.") : QString(Formatting::localDecimalPoint))
                                .arg(Formatting::localExponential)
                                .arg(Formatting::localPercent));

/**
 * Reload the application settings for formatting options.  Called when the
 * application is first started, and again whenever changes are made in the
 * Preferences dialog.
 */
void Formatting::updatePreferences(QSettings *settings)
{

}

/**
 * Interpret the provided string as a decimal value formatted as appropriate
 * for the current locale, and return the equivalent value for the "C"
 * locale.  Returns an empty string if the provided string isn't a valid
 * decimal value.
 *
 * @param value The input decimal value string
 * @return The equivalent string in the "C" locale, or "" if invalid
 */
QString Formatting::fromLocalDouble(const QString &value)
{
    bool ok = localRegExp.exactMatch(value);
    if (!ok) {
        return "";
    }
    QString result("");
    // parse the integral part and convert it; may be empty (e.g., ".12")
    QString integerPart = localRegExp.cap(1);
    if (!integerPart.isEmpty()) {
        int integer = systemLocale.toInt(integerPart, &ok);
        if (!ok) {
            return "";
        }
        result = cLocale.toString(integer);
    }
    QString decimalPart = localRegExp.cap(2);
    if (!decimalPart.isEmpty()) {
        result += decimalPart.replace(localDecimalPoint, '.');
    }
    QString exponentPart = localRegExp.cap(3);
    if (!exponentPart.isEmpty()) {
        result += exponentPart.replace(localExponential, 'E');
    }
    QString percentPart = localRegExp.cap(4);
    if (!percentPart.isEmpty()) {
        result += '%';
    }
    return result;
}

/**
 * Interpret the provided string as a decimal value formatted for the "C"
 * locale, and return the equivalent value for the current locale.  Returns
 * an empty string if the provided string isn't a valid decimal value.
 *
 * @param value The input decimal value string
 * @return The equivalent string in the current locale, or "" if invalid
 */
QString Formatting::toLocalDouble(const QString &value)
{
    bool ok = cRegExp.exactMatch(value);
    if (!ok) {
        return "";
    }
    QString result("");
    // parse the integral part and convert it; may be empty (e.g., ".12")
    QString integerPart = cRegExp.cap(1);
    if (!integerPart.isEmpty()) {
        int integer = cLocale.toInt(integerPart, &ok);
        if (!ok) {
            return "";
        }
        result = systemLocale.toString(integer);
    }
    QString decimalPart = cRegExp.cap(2);
    if (!decimalPart.isEmpty()) {
        result += decimalPart.replace('.', localDecimalPoint);
    }
    QString exponentPart = cRegExp.cap(3);
    if (!exponentPart.isEmpty()) {
        result += exponentPart.replace('E', localExponential);
    }
    QString percentPart = cRegExp.cap(4);
    if (!percentPart.isEmpty()) {
        result += localPercent;
    }
    return result;
}

/**
 * Get the double value corresponding to the C-locale number string provided.
 * Handles decimal points, group separators, exponentials, and percentages.
 *
 * @param value The input decimal value string
 * @param ok Pointer to a boolean to be set to true if the parsing succeeds
 *           or false otherwise
 */
double Formatting::parseDouble(const QString &value, bool *ok)
{
    QString temp(value);
    bool percentage = false;
    if (value.endsWith("%")) {
        temp.remove(temp.length() - 1, 1);
        percentage = true;
    }
    double result = cLocale.toDouble(temp, ok);
    if (percentage) {
        result = result / 100;
    }
    return result;
}

/**
 * Get the C-locale string representation of a floating point number.
 *
 * @param value The value to be formatted
 * @param decimals The number of decimal places to retain, -1 to leave it up
 *                 to the formatter
 * @return The formatted value
 */
QString Formatting::formatDouble(double value, int decimals)
{
    if (decimals == -1) {
        return cLocale.toString(value);
    }
    else {
        return cLocale.toString(value, 'f', decimals);
    }
}

/**
 * Interpret the provided string as an integer value formatted for the "C"
 * locale, and return the equivalent value for the current locale.  Returns
 * an empty string if the provided string isn't a valid integer.
 *
 * @param value The C-locale integer string to be converted
 * @return The equivalent string in the current locale, or "" if invalid
 */
QString Formatting::toLocalInt(const QString &value)
{
    bool ok;
    int integer = cLocale.toInt(value, &ok);
    if (!ok) {
        return "";
    }
    return systemLocale.toString(integer);
}
