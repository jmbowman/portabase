/*
 * qqutil/qqdialog.cpp
 *
 * (c) 2003-2009 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file qqutil/qqdialog.cpp
 * Source file for QQDialog
 */

#include <QApplication>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include "qqdialog.h"

/**
 * Constructor.
 * @param title The dialog caption; the application name will be used instead
 *              if this is empty, or simply appended otherwise.
 * @param parent The dialog's parent widget, if any.  Should usually be
 *               provided, modal dialogs and taskbar representation are a
 *               little odd otherwise.
 */
QQDialog::QQDialog(QString title, QWidget *parent)
    : QDialog(parent)
{
    if (title.isEmpty()) {
        setWindowTitle(qApp->applicationName());
    }
    else {
        setWindowTitle(title + " - " + qApp->applicationName());
    }
    setSizeGripEnabled(true);
}

/**
 * Convenience method for adding "OK" and "Cancel" buttons to the bottom of
 * a dialog which uses a QVBoxLayout.  The buttons are automatically linked
 * to the dialog's accept() and reject() slots.
 * @param layout The dialog's layout manager.
 * @param cancel False if the "Cancel" button is to be omitted.
 */
QDialogButtonBox *QQDialog::addOkCancelButtons(QVBoxLayout *layout, bool cancel)
{
    QDialogButtonBox::StandardButtons buttons;
    if (cancel) {
        buttons = QDialogButtonBox::Ok | QDialogButtonBox::Cancel;
    }
    else {
        buttons = QDialogButtonBox::Ok;
    }
    QDialogButtonBox *buttonBox = new QDialogButtonBox(buttons, Qt::Horizontal, this);
    layout->addWidget(buttonBox);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    if (cancel) {
        connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    }
    return buttonBox;
}

/**
 * Method to be called at the end of subclass constructors to handle window
 * geometry.
 * @param minWidth Minimum width in pixels.  If 0 or -1, don't set a minimum.
 * @param minHeight Minimum height in pixels.  If 0 or -1, don't set a minimum.
 */
void QQDialog::finishConstruction(int minWidth, int minHeight)
{
    if (minWidth != -1) {
        setMinimumWidth(minWidth);
    }
    if (minHeight != -1) {
        setMinimumHeight(minHeight);
    }
}
