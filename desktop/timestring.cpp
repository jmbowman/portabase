/*
 * timestring.cpp
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "timestring.h"
#include <qobject.h>
#include "config.h"

class TimeStringFormatKeeper
{
public:
    static PBDateFormat currentFormat()
    {
        self  = new TimeStringFormatKeeper;
	return self->format;
    }

private:
    static TimeStringFormatKeeper *self;
    PBDateFormat format;

    TimeStringFormatKeeper()
    {
	Config config("qpe");
	config.setGroup("Date");
	format = PBDateFormat(QChar(config.readEntry("Separator", "/")[0]),
		(PBDateFormat::Order)config.readNumEntry("ShortOrder", PBDateFormat::DayMonthYear), 
		(PBDateFormat::Order)config.readNumEntry("LongOrder", PBDateFormat::DayMonthYear));
    }
};

TimeStringFormatKeeper *TimeStringFormatKeeper::self = 0;

QString PBDateFormat::toNumberString() const
{
    QString buf = "";
    // for each part of the order
    for (int i = 0; i < 3; i++) {
	// switch on the relavent 3 bits.
	switch((_shortOrder >> (i * 3)) & 0x0007) { 
	    case 0x0001:
		buf += QObject::tr("D");
		break;
	    case 0x0002:
		buf += QObject::tr("M");
		break;
	    case 0x0004:
		buf += QObject::tr("Y");
		break;
	}
	if (i < 2) {
	    buf += _shortSeparator;
        }
    }
    return buf;
}

PBDateFormat TimeString::currentDateFormat()
{
    return TimeStringFormatKeeper::currentFormat();
}

QString TimeString::timeString( const QTime &t, bool ampm, bool seconds )
{
    if (!ampm) {
        if (seconds) {
	    return t.toString();
        }
	QString r = QString::number(t.hour());
	if (t.hour() < 10) {
            r.prepend("0");
        }
	r.append(":");
	if (t.minute() < 10) {
            r.append("0");
        }
	r.append(QString::number(t.minute()));
	return r;
    }
    // ### else the hard case that should disappear in Qt 3.0
    QString argString = seconds ? "%4:%5:%6 %7" : "%4:%5 %7";
    int hour = t.hour();
    QString strMin = QString::number(t.minute());
    QString strSec = QString::number(t.second());
    if (hour > 12) {
	argString = argString.arg(hour - 12, 2);
    }
    else {
        if (hour == 0) {
	    argString = argString.arg(12);
        }
        else {
	    argString = argString.arg(hour, 2);
        }
    }
    if (t.minute() < 10) {
	strMin.prepend("0");
    }
    if (t.second() < 10) {
	strSec.prepend("0");
    }
    argString = argString.arg(strMin);
    if (seconds) {
	argString = argString.arg(strSec);
    }
    if (hour >= 12) {
	argString = argString.arg(QObject::tr("PM"));
    }
    else {
	argString = argString.arg(QObject::tr("AM"));
    }
    return argString;
}

QString TimeString::timeString(const QTime &t, bool ampm)
{
    return timeString(t,ampm,FALSE);
}
