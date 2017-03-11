/*
 * qqspinbox.h
 *
 * (c) 2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file qqutil/qqspinbox.h
 * Header file for QQSpinBox
 */

#ifndef QQSPINBOX_H
#define QQSPINBOX_H

#include <QSpinBox>

#if !defined(ANDROID)
typedef QSpinBox QQSpinBox;
#else

/**
 * Subclass of QQSpinBox which prevents text selection of the current value
 * on button operations.  This is needed because on Android, the selection
 * markers are shown in the wrong place and a text operation toolbar appears
 * at the top of the screen, shifting the buttons down after the first press.
 */
class QQSpinBox: public QSpinBox
{
    Q_OBJECT
public:
    explicit QQSpinBox(QWidget *parent = 0);

    void stepBy(int steps);
    static void updateStyleSheet(QSpinBox *spinBox);
};
#endif // Q_OS_ANDROID
#endif // QQSPINBOX_H
