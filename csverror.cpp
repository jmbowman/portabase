/*
 * csverror.cpp
 *
 * (c) 2003-2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qlabel.h>
#include <qmultilineedit.h>
#include "csverror.h"

CSVErrorDialog::CSVErrorDialog(const QString &message, const QString &data, QWidget *parent, const char *name)
  : PBDialog("", parent, name)
{
    vbox->addWidget(new QLabel(message, this));
    vbox->addWidget(new QLabel(tr("Problematic row") + ":", this));

    QMultiLineEdit *dataBox = new QMultiLineEdit(this);
    dataBox->setWordWrap(QMultiLineEdit::NoWrap);
    dataBox->setText(data);
    dataBox->setReadOnly(TRUE);
    vbox->addWidget(dataBox);

    finishLayout(TRUE, FALSE, FALSE);
}

CSVErrorDialog::~CSVErrorDialog()
{

}
