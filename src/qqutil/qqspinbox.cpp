/*
 * qqspinbox.cpp
 *
 * (c) 2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file qqutil/qqspinbox.cpp
 * Source file for QQSpinBox
 */

#include "qqspinbox.h"

#if defined(Q_OS_ANDROID)

#include "qqfactory.h"

/**
 * Constructor.
 *
 * @param parent This spin box's parent widget
 */
QQSpinBox::QQSpinBox(QWidget *parent)
    : QSpinBox(parent)
{
    updateStyleSheet(this);
}

/**
 * Called whenever the user triggers a step.  Overridden to prevent the
 * text from being highlighted with each button click.
 *
 * @param steps Indicates how many steps were taken
 */
void QQSpinBox::stepBy(int steps)
{
    int old = value();
    setValue(old + steps * singleStep());
}

/**
 * Set the appropriate style sheet for the given spin box.  Takes the current
 * suffix (if any) into account.
 */
void QQSpinBox::updateStyleSheet(QSpinBox *spinBox)
{
    int pixels = QQFactory::dpToPixels(48);
    int minWidth = pixels * 3;
    if (!spinBox->suffix().isEmpty()) {
        minWidth += spinBox->fontMetrics().boundingRect(spinBox->suffix()).width();
    }
    QString sheet = QString("QSpinBox {min-height: %1px; min-width: %2px;}").arg(pixels).arg(minWidth) +
        QString("QSpinBox::down-button {subcontrol-position: left; width: %1px; height: %1px;}").arg(pixels) +
        QString("QSpinBox::up-button {subcontrol-position: right; width: %1px; height: %1px;}").arg(pixels) +
        QString("QSpinBox::up-arrow {image: url(:/icons/add.svg); width:%1px; height:%1px;}").arg(2 * pixels / 3) +
        QString("QSpinBox::down-arrow {image: url(:/icons/subtract.svg); width:%1px; height:%1px;}").arg(2 * pixels / 3);
    spinBox->setStyleSheet(sheet);
}

#endif
