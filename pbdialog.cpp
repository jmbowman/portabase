/*
 * pbdialog.cpp
 *
 * (c) 2003-2004,2008-2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file pbdialog.cpp
 * Source file for PBDialog
 */

#include <QApplication>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include "factory.h"
#include "pbdialog.h"

/**
 * Constructor.
 *
 * @param title The dialog caption; the application name will be used instead
 *              if this is empty, or simply appended otherwise
 * @param parent The dialog's parent widget, if any.  Should usually be
 *               provided, modal dialogs and taskbar representation are a
 *               little odd otherwise
 */
PBDialog::PBDialog(QString title, QWidget *parent)
    : QQDialog(title, parent)
{
    vbox = Factory::vBoxLayout(this, true);
}

/**
 * Finish setting up the dialog; add ok and/or cancel buttons, set minimum
 * dimensions if appropriate, set the layout, etc.  Called near the end of
 * most subclass constructors.
 *
 * @param okButton True if the dialog includes an "OK" button
 * @param cancelButton True if the dialog contains a "Cancel" button
 * @param minWidth Minimum width in pixels.  If 0 or -1, don't set a minimum.
 * @param minHeight Minimum height in pixels.  If 0 or -1, don't set a minimum.
 * @return The widget containing the OK/Cancel buttons, 0 if not added
 */
QDialogButtonBox *PBDialog::finishLayout(bool okButton, bool cancelButton,
                                         int minWidth, int minHeight)
{
    QDialogButtonBox *okCancelRow = 0;
    if (okButton || cancelButton) {
        okCancelRow = addOkCancelButtons(vbox, okButton, cancelButton);
    }
    finishConstruction(minWidth, minHeight);
    return okCancelRow;
}

/**
 * Add the standard buttons used on most list management dialogs: Up, Down,
 * and often Add, Edit, and Delete.  Subclasses are responsible for actually
 * connecting the button clicks to appropriate actions (using this class's
 * button attributes).
 *
 * @param movementOnly True if only the Up and Down buttons are to be added
 */
void PBDialog::addEditButtons(bool movementOnly)
{
    QHBoxLayout *hbox = Factory::hBoxLayout(vbox);
    if (!movementOnly) {
        addButton = new QPushButton(tr("Add"), this);
        hbox->addWidget(addButton);
        editButton = new QPushButton(tr("Edit"), this);
        hbox->addWidget(editButton);
        deleteButton = new QPushButton(tr("Delete"), this);
        hbox->addWidget(deleteButton);
    }
    upButton = new QPushButton(tr("Up"), this);
    hbox->addWidget(upButton);
    downButton = new QPushButton(tr("Down"), this);
    hbox->addWidget(downButton);
    // leave a blank space before the OK and Cancel buttons
    vbox->addWidget(new QLabel(" ", this));
}

/**
 * Perform standard validation checks on a provided name for an entity in
 * the database: column, filter, sorting, view, etc.  Prevents blank entries,
 * leading underscores, and duplicates.
 *
 * @param newName The proposed name
 * @param oldName The previous name, if this is a renaming
 * @param otherNames All names of other items of the same type
 * @return True if the name is acceptable, false otherwise
 */
bool PBDialog::validateName(const QString &newName, const QString &oldName,
                            const QStringList &otherNames)
{
    if (newName.isEmpty()) {
        QMessageBox::warning(this, qApp->applicationName(),
                             tr("No name entered"));
        return false;
    }
    if (newName == oldName) {
        // hasn't changed and isn't empty, must be valid
        return true;
    }
    if (newName[0] == '_') {
        QMessageBox::warning(this, qApp->applicationName(),
                             tr("Name must not start with '_'"));
        return false;
    }
    // check for other items with same name
    if (otherNames.contains(newName)) {
        QMessageBox::warning(this, qApp->applicationName(), tr("Duplicate name"));
        return false;
    }
    return true;
}
