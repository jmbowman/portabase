/*
 * numberwidget.cpp
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#if defined(DESKTOP)
#include "desktop/resource.h"
#else
#include <qpe/resource.h>
#endif

#include <qlineedit.h>
#include <qpushbutton.h>
#include "calculator.h"
#include "datatypes.h"
#include "numberwidget.h"

NumberWidget::NumberWidget(int type, QWidget *parent, const char *name, WFlags f)
  : QHBox(parent, name, f), dataType(type)
{
    entryField = new QLineEdit(this);
    QPushButton *button = new QPushButton(this);
    button->setPixmap(Resource::loadPixmap("portabase/calculator"));
    connect(button, SIGNAL(clicked()), this, SLOT(launchCalculator()));
    int height = button->height();
    setMaximumHeight(height);
    button->setMaximumWidth(height);
}

NumberWidget::~NumberWidget()
{

}

QString NumberWidget::getValue()
{
    return entryField->text();
}

void NumberWidget::setValue(const QString &value)
{
    if (dataType == INTEGER) {
        double result = value.toDouble();
        entryField->setText(QString::number((int)result));
    }
    else {
        entryField->setText(value);
    }
}

void NumberWidget::launchCalculator()
{
    Calculator calc(this);
    calc.setInitialValue(getValue());
    if (calc.exec()) {
        setValue(calc.result());
    }
}
