/*
 * qqfiledialog.h
 *
 * (c) 2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file qqutil/qqfiledialog.h
 * Header file for QQFileDialog
 */

#ifndef QQFILEDIALOG_H
#define QQFILEDIALOG_H

#include <QFileDialog>

#if !defined(ANDROID)
typedef QFileDialog QQFileDialog;
#else
#include <QScroller>

/**
 * Subclass of QFileDialog which works much better on Android.
 */
class QQFileDialog: public QFileDialog
{
    Q_OBJECT
public:
    explicit QQFileDialog(
            QWidget *parent = Q_NULLPTR, const QString &caption = QString(),
            const QString &dir = QString(), const QString &filter = QString());

    static QString getOpenFileName(
            QWidget *parent = Q_NULLPTR, const QString &caption = QString(),
            const QString &dir = QString(), const QString &filter = QString());
    static QString getSaveFileName(
            QWidget *parent = Q_NULLPTR, const QString &caption = QString(),
            const QString &dir = QString(), const QString &filter = QString());
    static QString defaultStoragePath();
    static QString sdCardPath();

private slots:
    void itemPressed(const QModelIndex &index);
    void itemReleased(const QModelIndex &index);
    void scrollerStateChanged(QScroller::State newState);

private:
    bool pressing; /**< Is an entry in the file listing being pressed, not scrolled? */
};
#endif // Q_OS_ANDROID
#endif // QQFILEDIALOG_H
