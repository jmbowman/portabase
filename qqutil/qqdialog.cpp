/*
 * qqutil/qqdialog.cpp
 *
 * (c) 2003-2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
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
#include <QSettings>
#include <QSize>
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
    : QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
    setWindowTitle(title);
}

/**
 * Destructor.  Saves the dialog's size, if it has changed.
 */
QQDialog::~QQDialog()
{
    int w = size().width();
    int h = size().height();
    if (w != minWidthSetting || h != minHeightSetting) {
        QSettings settings;
        QString widthName = QString("DialogSizes/%1Width").arg(dialogClassName);
        QString heightName = QString("DialogSizes/%1Height").arg(dialogClassName);
        settings.setValue(widthName, w);
        settings.setValue(heightName, h);
    }
}

/**
 * Overrides QWidget::setWindowTitle() to include the application name.
 *
 * @param title The dialog title, not including the application name
 */
void QQDialog::setWindowTitle(const QString &title)
{
    if (title.isEmpty()) {
        QWidget::setWindowTitle(qApp->applicationName());
    }
    else {
        QWidget::setWindowTitle(title + " - " + qApp->applicationName());
    }
}

/**
 *  Override QDialog's exec() method to show the dialog maximized on devices
 *  with small screens.
 */
int QQDialog::exec()
{

#if defined(Q_WS_HILDON) || defined(Q_WS_MAEMO_5)
    showMaximized();
    resize(parentWidget()->width(), height());
#endif
    return QDialog::exec();
}

/**
 * Convenience method for adding "OK" and "Cancel" buttons to the bottom of
 * a dialog which uses a QVBoxLayout.  The buttons are automatically linked
 * to the dialog's accept() and reject() slots.
 * @param layout The dialog's layout manager.
 * @param cancel False if the "Cancel" button is to be omitted.
 */
QDialogButtonBox *QQDialog::addOkCancelButtons(QVBoxLayout *layout, bool ok,
                                               bool cancel)
{
    QDialogButtonBox::StandardButtons buttons;
    if (ok && cancel) {
        buttons = QDialogButtonBox::Ok | QDialogButtonBox::Cancel;
    }
    else if (ok) {
        buttons = QDialogButtonBox::Ok;
    }
    else {
        buttons = QDialogButtonBox::Cancel;
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
 * geometry.  If this dialog has been launched before, sets the new dialog
 * to the same size as the last one.
 * @param minWidth Minimum width in pixels.  If 0 or -1, don't set a minimum.
 * @param minHeight Minimum height in pixels.  If 0 or -1, don't set a minimum.
 */
void QQDialog::finishConstruction(int minWidth, int minHeight)
{
    QSettings settings;
    dialogClassName = metaObject()->className();
    QString widthName = QString("DialogSizes/%1Width").arg(dialogClassName);
    QString heightName = QString("DialogSizes/%1Height").arg(dialogClassName);
    minWidthSetting = settings.value(widthName, minWidth).toInt();
    minHeightSetting = settings.value(heightName, minHeight).toInt();
    if (minWidthSetting != -1 && minHeightSetting != -1) {
        resize(minWidthSetting, minHeightSetting);
    }
}
