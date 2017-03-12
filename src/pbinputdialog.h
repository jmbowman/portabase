/*
 * pbinputdialog.h
 *
 * (c) 2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file pbinputdialog.h
 * Header file for PBInputDialog
 */

#include <QInputDialog>

#if !defined(Q_OS_ANDROID)
typedef QInputDialog PBInputDialog;
#else
#ifndef PBINPUTDIALOG_H
#define PBINPUTDIALOG_H

#include "pbdialog.h"
#include "qqutil/qqlineedit.h"

class QListWidget;

/**
 * Substitute for QInputDialog on Android.  Shows fullscreen with an
 * action bar, instead of appearing stealthily in the middle of the screen
 * without title or borders.
 */
class PBInputDialog: public PBDialog
{
public:
    PBInputDialog(const QString &title, QWidget *parent = Q_NULLPTR);

    static QString getItem(
            QWidget *parent, const QString &title,const QString &label,
            const QStringList &items, int current = 0, bool editable = true,
            bool *ok = Q_NULLPTR);
    static QString getText(
            QWidget *parent, const QString &title, const QString &label,
            QLineEdit::EchoMode mode = QLineEdit::Normal,
            const QString &text = QString(), bool *ok = Q_NULLPTR);

private:
    QQLineEdit *lineEdit;
    QListWidget *listWidget;
};
#endif // PBINPUTDIALOG_H
#endif // Q_OS_ANDROID
