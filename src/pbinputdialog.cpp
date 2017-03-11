/*
 * pbinputdialog.cpp
 *
 * (c) 2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file pbinputdialog.cpp
 * Source file for PBInputDialog
 */

#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>
#include "factory.h"
#include "pbinputdialog.h"
#include "qqutil/qqlineedit.h"

#if defined(Q_OS_ANDROID)

/**
 * Constructor.
 *
 * @param The text of the dialog's action bar
 * @param This dialog's parent widget, if any
 */
PBInputDialog::PBInputDialog(const QString &title, QWidget *parent)
    : PBDialog(title, parent, false, true), lineEdit(0), listWidget(0)
{

}

/**
 * Static convenience function to let the user select an item from a string
 * list.
 *
 * @param parent The dialog's parent widget
 * @param title The text which is displayed in the title bar of the dialog
 * @param label The text which is shown to the user (it should say what should
 *              be entered)
 * @param items The string list which is inserted into the list view
 * @param current The number of the item which should be the current item
 * @param editable Only false is supported for now
 * @param ok Will be set to true if the user pressed OK and to false if the
 *           user pressed Cancel
 * @return
 */
QString PBInputDialog::getItem(
        QWidget *parent, const QString &title, const QString &label,
        const QStringList &items, int current, bool editable, bool *ok)
{
    Q_UNUSED(editable)
    PBInputDialog *dialog = new PBInputDialog(title, parent);
    QLabel *labelWidget = new QLabel(label, dialog);
    labelWidget->setMargin(Factory::dpToPixels(6));
    dialog->vbox->addWidget(labelWidget);
    dialog->listWidget = Factory::listWidget(dialog);
    dialog->listWidget->setAlternatingRowColors(false);
    dialog->listWidget->addItems(items);
    dialog->listWidget->item(current)->setSelected(true);
    dialog->vbox->addWidget(dialog->listWidget);
    dialog->finishLayout();
    if (dialog->exec()) {
        if (dialog->listWidget->selectedItems().length() > 0) {
            *ok = true;
            return dialog->listWidget->selectedItems()[0]->text();
        }
    }
    *ok = false;
    return QString();
}

/**
 * Static convenience function to get a string from the user.
 *
 * @param parent The dialog's parent widget
 * @param title The text which is displayed in the title bar of the dialog
 * @param label The text which is shown to the user (it should say what should
 *              be entered)
 * @param mode The echo mode the line edit will use
 * @param text The default text which is placed in the line edit
 * @param ok Will be set to true if the user pressed OK and to false if the
 *           user pressed Cancel
 * @return
 */
QString PBInputDialog::getText(
        QWidget *parent, const QString &title, const QString &label,
        QLineEdit::EchoMode mode, const QString &text, bool *ok)
{
    PBInputDialog *dialog = new PBInputDialog(title, parent);
    QLabel *labelWidget = new QLabel(label, dialog);
    labelWidget->setMargin(Factory::dpToPixels(6));
    dialog->vbox->addWidget(labelWidget);
    dialog->lineEdit = new QQLineEdit(dialog);
    dialog->lineEdit->setText(text);
    dialog->lineEdit->setEchoMode(mode);
    dialog->lineEdit->setEnterKeyType(Qt::EnterKeyDone);
    dialog->vbox->addWidget(dialog->lineEdit);
    dialog->vbox->addStretch(1);
    dialog->finishLayout();
    if (dialog->exec()) {
        *ok = true;
        return dialog->lineEdit->text();
    }
    *ok = false;
    return QString();
}

#endif // Q_OS_ANDROID
