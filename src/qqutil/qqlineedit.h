/*
 * qqlineedit.h
 *
 * (c) 2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file qqutil/qqlineedit.h
 * Header file for QQLineEdit
 */


#include <QLineEdit>

#if !defined(ANDROID)
typedef QLineEdit QQLineEdit;
#else
#ifndef QQLINEEDIT_H
#define QQLINEEDIT_H

/**
 * Subclass of QLineEdit which allows specification of which enter key to
 * use on the Android software keyboard.  Shows a "Next" button by default,
 * which shifts focus to the next editable field.  (The QLineEdit default is
 * "Done", which exits the current dialog completely.)
 */
class QQLineEdit: public QLineEdit
{
    Q_OBJECT
public:
    explicit QQLineEdit(QWidget *parent = 0);

    QVariant inputMethodQuery(Qt::InputMethodQuery query) const;
    void setEnterKeyType(Qt::EnterKeyType type);

protected:
    virtual void keyPressEvent(QKeyEvent *e);

private:
    Qt::EnterKeyType enterKeyType; /**< The type of enter key to show in Android's software keyboard */
};
#endif // QQLINEEDIT_H
#endif // ANDROID
