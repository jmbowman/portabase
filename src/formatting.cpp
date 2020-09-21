/*
 * formatting.cpp
 *
 * (c) 2010-2011,2015-2016 by Jeremy Bowman <jmbowman@alum.mit.edu>
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
#include <QTextDocument>
#include <QTime>
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
QString Formatting::dateFormat;
QString Formatting::timeFormat;

/**
 * Reload the application settings for formatting options.  Called when the
 * application is first started, and again whenever changes are made in the
 * Preferences dialog.
 */
void Formatting::updatePreferences(QSettings *settings)
{
    dateFormat = settings->value("DateTime/ShortDateFormat", "yyyy-MM-dd").toString();
    timeFormat = settings->value("DateTime/TimeFormat", "hh:mm AP").toString();
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
        if (integer == 0 && integerPart[0] == '-') {
            result = "-0";
        }
        else {
            result = cLocale.toString(integer);
        }
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
        if (integer == 0 && integerPart[0] == '-') {
            result = "-0";
        }
        else {
            result = systemLocale.toString(integer);
        }
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
 * Determine if the provided date is set to the PortaBase "None" date.  This
 * value is set to 1752-09-14, which used to be the earliest date supported
 * by Qt.
 *
 * @param date The date to be tested
 * @return True if the provided value is the "None" date, false otherwise
 */
bool Formatting::isNoneDate(const QDate &date)
{
    return (date.year() == 1752 && date.month() == 9 && date.day() == 14);
}

/**
 * Get a text representation of the date represented by the given integer.
 * (PortaBase stores a date YYYY-MM-DD as the integer with digits YYYYMMDD.)
 *
 * @param date The integer-encoded date to be formatted
 * @return The formatted date
 */
QString Formatting::dateToString(int date)
{
    int y = date / 10000;
    int m = (date - y * 10000) / 100;
    int d = date - y * 10000 - m * 100;
    QDate dateObj(y, m, d);
    return dateToString(dateObj);
}

/**
 * Get a text representation of the given date.
 *
 * @param date The date to be formatted
 * @return The formatted date
 */
QString Formatting::dateToString(const QDate &date)
{
    if (isNoneDate(date)) {
        return "";
    }
    return date.toString(dateFormat);
}

/**
 * Get a human-readable time string from the given number of seconds past
 * midnight.
 *
 * @param time The time to be converted (counted in seconds past midnight)
 * @return A human-readable time string
 */
QString Formatting::timeToString(int time)
{
    if (time == -1) {
        return "";
    }
    QTime midnight(0, 0);
    QTime timeObj = midnight.addSecs(time);
    return timeObj.toString(timeFormat);
}

/**
 * Get a human-readable time string from the given date-time object.
 *
 * @param dateTime The date and time to be formatted
 * @return A human-readable date + time string
 */
QString Formatting::dateTimeToString(const QDateTime &dateTime)
{
    QString date = dateToString(dateTime.date());
    QString time = dateTime.time().toString(timeFormat);
    return QString("%1 %2").arg(date).arg(time);
}

/**
 * Parse a time value from the provided string.
 *
 * @param value The text to be parsed
 * @param ok Pointer to a boolean value which will represent the success or
 *           failure of the parsing attempt
 * @return The number of seconds after midnight in the parsed time, shown as
 *         a string
 */
QString Formatting::parseTimeString(const QString &value, bool *ok)
{
    // check for imported blank
    if (value.isEmpty()) {
        *ok = true;
        return "-1";
    }
    int length = value.length();
    int firstColon = value.indexOf(':');
    if (firstColon == -1) {
        // assume it's a number of seconds, as used internally
        int totalSeconds = value.toInt(ok);
        if (!(*ok) || totalSeconds < -1 || totalSeconds > 86399) {
            *ok = false;
        }
        return value;
    }
    // from here on is used only when importing
    if (firstColon < 1 || length < firstColon + 3) {
        *ok = false;
        return value;
    }
    int hours = value.left(firstColon).toInt(ok);
    if (!(*ok)) {
        return value;
    }
    int minutes = value.mid(firstColon + 1, 2).toInt(ok);
    if (!(*ok)) {
        return value;
    }
    int seconds = 0;
    int secondColon = value.indexOf(':', firstColon + 1);
    if (secondColon != -1 && length > secondColon + 2) {
        seconds = value.mid(secondColon + 1, 2).toInt(ok);
        if (!(*ok)) {
            return value;
        }
    }
    if (value.indexOf("pm", 0, Qt::CaseInsensitive) != -1) {
        if (hours < 12) {
            hours += 12;
        }
    }
    else if (value.indexOf("am", 0, Qt::CaseInsensitive) != -1 && hours == 12) {
        hours = 0;
    }
    QTime time;
    if (!time.setHMS(hours, minutes, seconds)) {
        *ok = false;
        return value;
    }
    QTime midnight(0, 0);
    int totalSeconds = midnight.secsTo(time);
    *ok = true;
    return QString::number(totalSeconds);
}

/**
 * Get a copy of the provided string with HTML/XML special characters (such as
 * '<', '>', '&', and '"') escaped.  Abstracts away the different APIs for doing
 * this in Qt 4 and 5.
 *
 * @param value The string to be escaped
 * @return A copy of the input string with special characters escaped
 */
QString Formatting::toHtmlEscaped(const QString &value)
{
#if QT_VERSION >= 0x050000
  return value.toHtmlEscaped();
#else
  return Qt::escape(value);
#endif
}
