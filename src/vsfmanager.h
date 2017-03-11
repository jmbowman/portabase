/*
 * vsfmanager.h
 *
 * (c) 2010,2015,2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file vsfmanager.h
 * Header file for VSFManager
 */

#ifndef VSFMANAGER_H
#define VSFMANAGER_H

#include <QList>
#include "pbdialog.h"

class Database;
class PortaBase;
class QAbstractButton;
class QLabel;

/**
 * Dialog for managing the current View, Sorting, or Filter selection (hence
 * "VSF").  Launched by a toolbar button, this dialog contains button to edit
 * or delete the current item, add a new item, or select an existing item.
 */
class VSFManager: public PBDialog
{
    Q_OBJECT
public:
    /** Enumeration of dialog subject types */
    enum Subject {
        View = 0,
        Sorting = 1,
        Filter = 2
    };

    explicit VSFManager(PortaBase *parent = 0);

    void setSubject(Database *database, Subject s);
    void setActions(QAction *add, QAction *edit, QAction *del);
    void setOptions(const QString &current, const QStringList &all);

private:
    void updateButtonList(const QStringList &items, const QString &current);
    QString displayedName(const QString &name, bool *specialCase);

private slots:
    void addItem();
    void editItem();
    void deleteItem();
    void selectItem();

private:
    PortaBase *portabase; /**< The main application window */
    Database *db; /**< The database in use */
    Subject subject; /**< The type of item being managed */
    QVBoxLayout *layout; /**< The layout containing the button list */
    QLabel *currentLabel; /**< The "Current..." text label */
    QAbstractButton *editButton; /**< The edit item button */
    QAbstractButton *deleteButton; /**< The delete item button */
    QPushButton *addButton; /**< The add item button */
    QList<QPushButton*> buttonList; /**< The buttons for selecting existing items */
    QRegExp nameFilter; /**< Regular expression for filtering out special cases */
#if defined(Q_OS_ANDROID)
    QAction *editButtonAction; /**< Action directly triggered by the edit item button */
    QAction *deleteButtonAction; /**< Action directly triggered by the delete item button */
#endif
};

#endif // VSFMANAGER_H
