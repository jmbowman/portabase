/*
 * pbdialog.cpp
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
#endif

#include <qhbox.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include "pbdialog.h"

PBDialog::PBDialog(QString title, QWidget *parent, const char *name, WFlags f)
    : QDialog(parent, name, TRUE, f), okCancelRow(0)
{
    vbox = new QVBoxLayout(this);
    if (title.isEmpty()) {
        setCaption(tr("PortaBase") + PBDialog::titleSuffix);
    }
    else {
        setCaption(title + " - " + tr("PortaBase") + PBDialog::titleSuffix);
    }
#if defined(Q_WS_WIN)
    setSizeGripEnabled(TRUE);
    vbox->setMargin(8);
#endif
}

#if defined(Q_WS_WIN)
// non-commercial Qt doesn't show the titlebar text unless this is present
const QString PBDialog::titleSuffix = " - Qt";
#else
const QString PBDialog::titleSuffix = "";
#endif

PBDialog::~PBDialog()
{

}

void PBDialog::finishLayout(bool okButton, bool cancelButton, bool fullscreen,
                            int minWidth, int minHeight)
{
#if defined(DESKTOP)
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
    QWidget *parent = parentWidget();
    if (fullscreen) {
        int width = (minWidth == -1) ? parent->width() / 2 : minWidth;
        int height = (minHeight == -1) ? parent->height() : minHeight;
        setMinimumWidth(width);
        setMinimumHeight(height);
    }
    else {
        if (minWidth != -1) {
            setMinimumWidth(minWidth);
        }
        if (minHeight != -1) {
            setMinimumHeight(minHeight);
        }
    }
    setIcon(Resource::loadPixmap("portabase"));
#else
    if (fullscreen) {
        showMaximized();
    }
#endif
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
#if defined(DESKTOP)
    // leave a blank space before the OK and Cancel buttons
    vbox->addWidget(new QLabel(" ", this));
#endif
}

bool PBDialog::validateName(const QString &newName, const QString &oldName,
                            const QStringList &otherNames)
{
    if (newName.isEmpty()) {
        QMessageBox::warning(this, tr("PortaBase"),
                             tr("No name entered"));
        return FALSE;
    }
    if (newName == oldName) {
        // hasn't changed and isn't empty, must be valid
        return TRUE;
    }
    if (newName[0] == '_') {
        QMessageBox::warning(this, tr("PortaBase"),
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
        QMessageBox::warning(this, tr("PortaBase"), tr("Duplicate name"));
    }
    return result;
}
