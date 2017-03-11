/*
 * qqutil/qqdialog.cpp
 *
 * (c) 2003-2012,2016-2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
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
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QSettings>
#include <QSize>
#include <QVBoxLayout>
#include "qqdialog.h"
#include "qqfactory.h"

#if defined(Q_OS_ANDROID)
#include <QScreen>
#include "actionbar.h"
#endif

/**
 * Constructor.
 * @param title The dialog caption; the application name will be used instead
 *              if this is empty, or simply appended otherwise.
 * @param parent The dialog's parent widget, if any.  Should usually be
 *               provided, modal dialogs and taskbar representation are a
 *               little odd otherwise.
 * @param small True if the dialog is small enough that it shouldn't be
 *              maximized on platforms with small screens (like Android and
 *              Maemo)
 * @param backButtonAccepts True if the Android back button applies changes
 *                          made in the dialog
 */
QQDialog::QQDialog(QString title, QWidget *parent, bool small, bool backButtonAccepts)
    : QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint),
    smallDialog(small), acceptOnBack(backButtonAccepts)
{
#if defined(Q_WS_MAEMO_5)
    QHBoxLayout *hbox = QQFactory::hBoxLayout(this, true);
    vbox = QQFactory::vBoxLayout(hbox);
#else
    vbox = QQFactory::vBoxLayout(this, true);
#endif
#if defined(Q_OS_ANDROID)
    actionBar = 0;
    screenGeometryChanged(qApp->primaryScreen()->availableGeometry());
    connect(qApp->primaryScreen(), SIGNAL(availableGeometryChanged(const QRect &)),
            this, SLOT(screenGeometryChanged(const QRect &)));
    actionBar = new ActionBar(this);
    vbox->addWidget(actionBar);
    if (backButtonAccepts) {
        connect(actionBar, &ActionBar::up, this, &QQDialog::accept);
    }
    else {
        connect(actionBar, &ActionBar::up, this, &QQDialog::reject);
    }
#endif
    setWindowTitle(title);
#if defined(Q_WS_MAEMO_5)
    if (!small) {
        setAttribute(Qt::WA_Maemo5StackedWindow);
        setWindowFlags(Qt::Window);
    }
#elif defined(Q_OS_ANDROID)
    setWindowFlags(Qt::Window);
#endif
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
 * Overrides QWidget::setWindowTitle() to include the application name. Qt 5
 * does this automatically when appropriate on most platforms, so this is
 * mainly for the benefit of Maemo and Android (where it puts the title in
 * the action bar).
 *
 * @param title The dialog title, not including the application name
 */
void QQDialog::setWindowTitle(const QString &title)
{
#if defined(Q_OS_ANDROID)
    actionBar->setTitle(title, true);
#elif QT_VERSION >= 0x050000
    QWidget::setWindowTitle(title);
#else
    if (title.isEmpty()) {
        QWidget::setWindowTitle(qApp->applicationName());
    }
    else {
        QWidget::setWindowTitle(title + " - " + qApp->applicationName());
    }
#endif
}

/**
 *  Override QDialog's exec() method to show the dialog maximized on devices
 *  with small screens.
 */
int QQDialog::exec()
{
#if defined(Q_WS_HILDON) || defined(Q_WS_MAEMO_5)
    if (!smallDialog) {
        showMaximized();
        resize(parentWidget()->width(), height());
    }
#endif
#if defined(Q_OS_ANDROID)
    showMaximized();
#endif
    return QDialog::exec();
}

/**
 * Convenience method for adding "OK" and "Cancel" buttons to the bottom of
 * a dialog which uses a QVBoxLayout.  The buttons are automatically linked
 * to the dialog's accept() and reject() slots.
 *
 * @param layout The dialog's layout manager.
 * @param ok False if the "OK" button is to be omitted
 * @param cancel False if the "Cancel" button is to be omitted.
 */
QDialogButtonBox *QQDialog::addOkCancelButtons(QBoxLayout *layout, bool ok,
                                               bool cancel)
{
    QDialogButtonBox::StandardButtons buttons;
    if (ok && cancel) {
        buttons = QDialogButtonBox::Ok | QDialogButtonBox::Cancel;
    }
#if defined(Q_OS_ANDROID)
    else {
        return 0;
    }
#else
    else if (ok) {
        buttons = QDialogButtonBox::Ok;
    }
    else {
        buttons = QDialogButtonBox::Cancel;
    }
#endif
    Qt::Orientation orientation = Qt::Horizontal;
    QBoxLayout::Direction dir = layout->direction();
    if (dir == QBoxLayout::LeftToRight || dir == QBoxLayout::RightToLeft) {
        orientation = Qt::Vertical;
    }
    QDialogButtonBox *buttonBox = new QDialogButtonBox(buttons, orientation, this);
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
#ifndef MOBILE
    QSettings settings;
    dialogClassName = metaObject()->className();
    QString widthName = QString("DialogSizes/%1Width").arg(dialogClassName);
    QString heightName = QString("DialogSizes/%1Height").arg(dialogClassName);
    minWidthSetting = settings.value(widthName, minWidth).toInt();
    minHeightSetting = settings.value(heightName, minHeight).toInt();
    if (minWidthSetting != -1 && minHeightSetting != -1) {
        resize(minWidthSetting, minHeightSetting);
    }
#else
    Q_UNUSED(minWidth);
    Q_UNUSED(minHeight);
#endif
}

#if defined(Q_OS_ANDROID)
/**
 * If Android's back button is pressed, treat it as accepting any changes
 * made in the dialog.
 *
 * @param e A key press event object
 */
void QQDialog::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Back) {
        if (acceptOnBack) {
            accept();
        }
        else {
            reject();
        }
        return;
    }
    QDialog::keyPressEvent(e);
}
#endif

/**
 * Respond to the screen orientation or size changing.  Used on Android to
 * make sure the dialog width doesn't exceed the screen width.
 */
void QQDialog::screenGeometryChanged(const QRect &geometry)
{
#if defined(Q_OS_ANDROID)
    if (actionBar) {
        actionBar->setMaximumWidth(geometry.width());
    }
    if (isVisible()) {
        setMaximumWidth(geometry.width());
        setMinimumHeight(geometry.height());
    }
#else
    Q_UNUSED(geometry)
#endif
}
