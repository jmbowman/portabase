/*
 * qqutil/qqdialog.h
 *
 * (c) 2003-2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file qqutil/qqdialog.h
 * Header file for QQDialog
 */

#ifndef QQDIALOG_H
#define QQDIALOG_H

#include <qdialog.h>

class QDialogButtonBox;
class QBoxLayout;

/**
 * Base class for most dialogs in an application.  By calling
 * finishConstruction() at the end of subclass constructors, minimum
 * dimensions can be set intelligently.
 */
class QQDialog: public QDialog
{
    Q_OBJECT
public:
    QQDialog(QString title, QWidget *parent=0, bool small=false);
    virtual ~QQDialog();

    void setWindowTitle(const QString &title);

public slots:
    int exec();

protected:
    QDialogButtonBox *addOkCancelButtons(QBoxLayout *layout, bool ok=true,
                                         bool cancel=true);
    void finishConstruction(int minWidth=-1, int minHeight=-1);

private:
    QString dialogClassName; /**< The class name of the subclass instance */
    int minWidthSetting; /**< The last loaded minimum width setting */
    int minHeightSetting; /**< The last loaded minimum height setting */
    bool smallDialog; /**< True if the dialog is particularly small */
};

#endif
