/*
 * numberwidget.cpp
 *
 * (c) 2003-2004,2008-2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file numberwidget.cpp
 * Source file for NumberWidget
 */

#include <QAbstractButton>
#include <QIcon>
#include <QLayout>
#include <QLineEdit>
#include <QLocale>
#include "calculator.h"
#include "datatypes.h"
#include "factory.h"
#include "formatting.h"
#include "numberwidget.h"

/**
 * Constructor.
 *
 * @param type The type of value to edit (FLOAT or INT)
 * @param parent This widget's parent widget
 */
NumberWidget::NumberWidget(int type, QWidget *parent)
  : QWidget(parent), dataType(type)
{
    QHBoxLayout *layout = Factory::hBoxLayout(this, true);
    entryField = new QLineEdit(this);
    layout->addWidget(entryField, 1);
    QAbstractButton *button = Factory::button(this);
    layout->addWidget(button);
    button->setIcon(QIcon(":/icons/calculator.png"));
    button->setToolTip(tr("Show calculator"));
    connect(button, SIGNAL(clicked()), this, SLOT(launchCalculator()));
}

/**
 * Get the currently selected value.
 *
 * @return The C-locale text representation of the currently selected value
 */
QString NumberWidget::getValue()
{
    if (dataType == INTEGER) {
        int value = QLocale::system().toInt(entryField->text());
        return QString::number(value);
    }
    else {
        return Formatting::fromLocalDouble(entryField->text());
    }
}

/**
 * Set this widget's new value.
 *
 * @param value The new value to be displayed in the widget
 */
void NumberWidget::setValue(const QString &value)
{
    if (dataType == INTEGER) {
        double result = Formatting::parseDouble(value);
        entryField->setText(QLocale::system().toString((int)result));
    }
    else {
        entryField->setText(Formatting::toLocalDouble(value));
    }
}

/**
 * Launch the calculator dialog.  Called automatically when the right-hand
 * button is clicked.
 */
void NumberWidget::launchCalculator()
{
    Calculator calc(this);
    calc.setInitialValue(getValue());
    if (calc.exec()) {
        setValue(calc.result());
    }
}
