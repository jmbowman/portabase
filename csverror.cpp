/*
 * csverror.cpp
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#if defined(DESKTOP)
#include <qhbox.h>
#include <qpushbutton.h>
#include "desktop/resource.h"
#endif

#include <qlabel.h>
#include <qlayout.h>
#include <qmultilineedit.h>
#include "csverror.h"

CSVErrorDialog::CSVErrorDialog(const QString &message, const QString &data, QWidget *parent, const char *name, WFlags f) : QDialog(parent, name, TRUE, f)
{
    setCaption(tr("PortaBase"));
    QVBoxLayout *vbox = new QVBoxLayout(this);
#if defined(Q_WS_WIN)
    setSizeGripEnabled(TRUE);
    vbox->setMargin(8);
#endif

    vbox->addWidget(new QLabel(message, this));
    vbox->addWidget(new QLabel(tr("Problematic row") + ":", this));

    QMultiLineEdit *dataBox = new QMultiLineEdit(this);
    dataBox->setWordWrap(QMultiLineEdit::NoWrap);
    dataBox->setText(data);
    dataBox->setReadOnly(TRUE);
    vbox->addWidget(dataBox);

#if defined(DESKTOP)
    QHBox *hbox = new QHBox(this);
    new QWidget(hbox);
    QPushButton *okButton = new QPushButton(tr("OK"), hbox);
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    new QWidget(hbox);
    vbox->addWidget(hbox);
    vbox->setResizeMode(QLayout::FreeResize);
    setIcon(Resource::loadPixmap("portabase"));
#endif
}

CSVErrorDialog::~CSVErrorDialog()
{

}
