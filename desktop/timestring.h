/*
 * timestring.h
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef _TIMESTRING_H_
#define _TIMESTRING_H_
#include <qdatetime.h>
#include <qstring.h>

class PBDateFormat 
{
public:
    // date format type 001,010,100 = day month year
    enum Order {
	DayMonthYear = 0x0111, // 0x001 + 0x010(0x2 << 3) + 0x100(0x4 << 3)
	MonthDayYear = 0x010A,
	YearMonthDay = 0x0054
    };

    PBDateFormat(QChar s = '/', Order so = MonthDayYear) : _shortOrder(so),
        _longOrder(so), _shortSeparator(s) { }
    PBDateFormat(QChar s, Order so, Order lo) :  _shortOrder(so),
        _longOrder(lo), _shortSeparator(s) { }
    PBDateFormat(const PBDateFormat &o) : _shortOrder(o._shortOrder),
        _longOrder(o._longOrder), _shortSeparator(o._shortSeparator) { }

    bool operator==(const PBDateFormat &o)
    {
	if (o._shortOrder == _shortOrder && o._longOrder == _longOrder &&
                o._shortSeparator == _shortSeparator) {
	    return TRUE;
        }
	return FALSE;
    }

    QString toNumberString() const; // the M/D/Y string.

    QChar separator() const { return _shortSeparator; };
    Order shortOrder() const { return _shortOrder; };
    Order longOrder() const { return _longOrder; };

private:
    Order _shortOrder;
    Order _longOrder;
    QChar _shortSeparator;
};

class TimeString
{
public:
    static QString timeString(const QTime &t, bool ampm, bool seconds);
    static QString timeString(const QTime &t, bool ampm = false);
    static QString shortTime(bool ampm, bool seconds);
    static QString shortTime(bool ampm = false);
    static PBDateFormat currentDateFormat();
};
#endif
