/*
 * pbdialog.cpp
 *
 * (c) 2003-2004,2008-2010,2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file pbdialog.cpp
 * Source file for PBDialog
 */

#include <QAction>
#include <QApplication>
#include <QDialogButtonBox>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include "pbdialog.h"
#include "qqutil/qqfactory.h"
#if defined(Q_OS_ANDROID)
#include "qqutil/actionbar.h"
#endif

/**
 * Constructor.
 *
 * @param title The dialog caption; the application name will be used instead
 *              if this is empty, or simply appended otherwise
 * @param parent The dialog's parent widget, if any.  Should usually be
 *               provided, modal dialogs and taskbar representation are a
 *               little odd otherwise
 * @param small True if the dialog is small enough that it shouldn't be
 *              maximized on platforms with small screens (like Maemo)
 * @param backButtonAccepts True if the Android back button applies changes
 *                          made in the dialog
 */
PBDialog::PBDialog(QString title, QWidget *parent, bool small, bool backButtonAccepts)
    : QQDialog(title, parent, small, backButtonAccepts), addAction(0),
      editAction(0), deleteAction(0), upAction(0), downAction(0)
{

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
    okCancelRow = addOkCancelButtons(static_cast<QBoxLayout *>(layout()),
                                     okButton, cancelButton);
#if defined(Q_WS_HILDON) || defined(Q_WS_MAEMO_5)
    if (addAction) {
        okCancelRow->addButton(actionButtonMap[addAction],
                               QDialogButtonBox::ActionRole);
        okCancelRow->addButton(actionButtonMap[editAction],
                               QDialogButtonBox::ActionRole);
        okCancelRow->addButton(actionButtonMap[deleteAction],
                               QDialogButtonBox::ActionRole);
    }
    if (upAction) {
        okCancelRow->addButton(actionButtonMap[upAction],
                               QDialogButtonBox::ActionRole);
        okCancelRow->addButton(actionButtonMap[downAction],
                               QDialogButtonBox::ActionRole);
    }
#endif
    finishConstruction(minWidth, minHeight);
    return okCancelRow;
}

/**
 * Add the standard buttons used on most list management dialogs: Up, Down,
 * and often Add, Edit, and Delete.  Subclasses are responsible for actually
 * connecting the button clicks to appropriate slots (using this class's
 * action attributes).
 *
 * @param movementOnly True if only the Up and Down buttons are to be added
 */
void PBDialog::addEditButtons(bool movementOnly)
{
#ifndef MOBILE
    QHBoxLayout *hbox = QQFactory::hBoxLayout(vbox);
    // leave a blank space before the OK and Cancel buttons
    vbox->addWidget(new QLabel(" ", this));
#else
    QHBoxLayout *hbox = 0;
#endif
    if (!movementOnly) {
        addAction = new QAction(QQFactory::icon("add"), tr("Add"), this);
        addButton(addAction, hbox);
        editAction = new QAction(QQFactory::icon("edit"), tr("Edit"), this);
        addButton(editAction, hbox);
        deleteAction = new QAction(QQFactory::icon("delete"), tr("Delete"), this);
        addButton(deleteAction, hbox);
    }
    upAction = new QAction(QQFactory::icon("up"), tr("Up"), this);
    addButton(upAction, hbox);
    downAction = new QAction(QQFactory::icon("down"), tr("Down"), this);
    addButton(downAction, hbox);
}

/**
 * Add a button that will perform the given dialog action.
 *
 * @param action The action for which to create a button
 * @param hbox The layout row to which the button should be added on
 *             non-mobile platforms
 */
void PBDialog::addButton(QAction *action, QHBoxLayout *hbox)
{
#if defined(Q_OS_ANDROID)
    Q_UNUSED(hbox)
    actionBar->addButton(action);
#else
    QPushButton *button = new QPushButton(action->text(), this);
    actionButtonMap[action] = button;
    connect(button, SIGNAL(clicked()), action, SIGNAL(triggered()));
    connect(action, SIGNAL(changed()), this, SLOT(actionChanged()));
    if (hbox) {
        hbox->addWidget(button);
    }
#endif
}

/**
 * Update the corresponding push button (if any) when one of the standard
 * dialog actions is enabled or disabled.
 */
void PBDialog::actionChanged()
{
#if !defined(Q_OS_ANDROID)
    QAction *action = qobject_cast<QAction *>(sender());
    if (actionButtonMap.contains(action)) {
        actionButtonMap[action]->setEnabled(action->isEnabled());
    }
#endif
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
