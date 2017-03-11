/*
 * qqlineedit.cpp
 *
 * (c) 2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file qqutil/qqlineedit.cpp
 * Source file for QQLineEdit
 */

#include "qqlineedit.h"

#if defined(Q_OS_ANDROID)

#include <QApplication>
#include <QKeyEvent>

/**
 * Constructor.
 *
 * @param parent This edit field's parent widget
 */
QQLineEdit::QQLineEdit(QWidget *parent)
    : QLineEdit(parent), enterKeyType(Qt::EnterKeyNext)
{

}

/**
 * This method is used by the input method to query a set of properties of
 * the widget to be able to support complex input method operations such as
 * support for surrounding text and reconversions.  Overridden to show the
 * correct action button in Android's software keyboard.
 *
 * @param query The property being queried
 * @return The value of the requested property
 */
QVariant QQLineEdit::inputMethodQuery(Qt::InputMethodQuery query) const
{
    if (query == Qt::ImEnterKeyType) {
        return enterKeyType;
    }
    return QLineEdit::inputMethodQuery(query);
}

/**
 * If Android's next button is pressed, shift focus to the next available
 * field.  If that field can contain characters, go ahead and launch the
 * software keyboard again.
 *
 * @param e A key press event object
 */
void QQLineEdit::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Return && enterKeyType == Qt::EnterKeyNext) {
        if (focusNextChild()) {
            QWidget *nextWidget = qApp->focusWidget();
            if (nextWidget->inputMethodQuery(Qt::ImCurrentSelection).isValid()) {
                QEvent event(QEvent::RequestSoftwareInputPanel);
                qApp->sendEvent(nextWidget, &event);
            }
        }
        return;
    }
    QLineEdit::keyPressEvent(e);
}

/**
 * Set the type of enter key to use on Android's software keyboard.
 *
 * @param type The type of enter key to use
 */
void QQLineEdit::setEnterKeyType(Qt::EnterKeyType type)
{
    enterKeyType = type;
}

#endif // Q_OS_ANDROID
