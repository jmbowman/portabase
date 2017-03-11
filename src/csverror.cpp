/*
 * csverror.cpp
 *
 * (c) 2003-2004,2008-2012,2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file csverror.cpp
 * Source file for CSVErrorDialog
 */

#include <QLabel>
#include <QLayout>
#include <QTextEdit>
#include "csverror.h"
#include "qqutil/qqfactory.h"

#if defined(Q_WS_MAEMO_5)
#include <QAbstractKineticScroller>
#endif

/**
 * Constructor.
 *
 * @param message The error message to be displayed
 * @param data The line of CSV data which triggered the error
 * @param parent The parent widget, if any
 */
CSVErrorDialog::CSVErrorDialog(const QString &message, const QString &data, QWidget *parent)
  : PBDialog("", parent)
{
    vbox->addWidget(new QLabel(message, this));
    vbox->addWidget(new QLabel(tr("Problematic row") + ":", this));

    QTextEdit *dataBox = QQFactory::textEdit(parent);
    dataBox->setReadOnly(true);
#if defined(Q_WS_MAEMO_5)
    QVariant ksProp = dataBox->property("kineticScroller");
    QAbstractKineticScroller *ks = ksProp.value<QAbstractKineticScroller *>();
    if (ks) {
        ks->setEnabled(true);
    }
#endif
    dataBox->setLineWrapMode(QTextEdit::NoWrap);
    dataBox->setPlainText(data);
    vbox->addWidget(dataBox);

    finishLayout(true, false);
}
