/*
 * pbdialog.cpp
 *
 * (c) 2003-2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qhbox.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include "pbdialog.h"

PBDialog::PBDialog(QString title, QWidget *parent, const char *name, WFlags f)
    : QQDialog(title, parent, name, TRUE, f), okCancelRow(0)
{
    vbox = new QVBoxLayout(this);
#if defined(Q_WS_WIN)
    setSizeGripEnabled(TRUE);
    vbox->setMargin(8);
#endif
}

PBDialog::~PBDialog()
{

}

void PBDialog::finishLayout(bool okButton, bool cancelButton, bool fullscreen,
                            int minWidth, int minHeight)
{
#if !defined(Q_WS_QWS)
    if (okButton || cancelButton) {
        QHBox *hbox = new QHBox(this);
        vbox->addWidget(hbox);
        new QWidget(hbox);
        if (okButton) {
            QPushButton *okButton = new QPushButton(tr("OK"), hbox);
            connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
            new QWidget(hbox);
        }
        if (cancelButton) {
            QPushButton *cancelButton = new QPushButton(tr("Cancel"), hbox);
            connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
            new QWidget(hbox);
        }
        vbox->setResizeMode(QLayout::FreeResize);
        okCancelRow = hbox;
    }
#endif
    finishConstruction(fullscreen, minWidth, minHeight);
}

void PBDialog::addEditButtons(bool movementOnly)
{
    QHBox *hbox = new QHBox(this);
    vbox->addWidget(hbox);
    if (!movementOnly) {
        addButton = new QPushButton(tr("Add"), hbox);
        editButton = new QPushButton(tr("Edit"), hbox);
        deleteButton = new QPushButton(tr("Delete"), hbox);
    }
    upButton = new QPushButton(tr("Up"), hbox);
    downButton = new QPushButton(tr("Down"), hbox);
#if !defined(Q_WS_QWS)
    // leave a blank space before the OK and Cancel buttons
    vbox->addWidget(new QLabel(" ", this));
#endif
}

bool PBDialog::validateName(const QString &newName, const QString &oldName,
                            const QStringList &otherNames)
{
    if (newName.isEmpty()) {
        QMessageBox::warning(this, QQDialog::tr("PortaBase"),
                             tr("No name entered"));
        return FALSE;
    }
    if (newName == oldName) {
        // hasn't changed and isn't empty, must be valid
        return TRUE;
    }
    if (newName[0] == '_') {
        QMessageBox::warning(this, QQDialog::tr("PortaBase"),
                             tr("Name must not start with '_'"));
        return FALSE;
    }
    // check for other items with same name
    bool result = TRUE;
    int count = otherNames.count();
    for (int i = 0; i < count; i++) {
        if (newName == otherNames[i]) {
            result = FALSE;
            break;
        }
    }
    if (!result) {
        QMessageBox::warning(this, QQDialog::tr("PortaBase"), tr("Duplicate name"));
    }
    return result;
}
