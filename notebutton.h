/*
 * notebutton.h
 *
 * (c) 2002,2009 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file notebutton.h
 * Header file for NoteButton
 */

#ifndef NOTEBUTTON_H
#define NOTEBUTTON_H

#include <QToolButton>

/**
 * Button which represents a note field.  Displays the beginning of the note
 * text, and launches an editor for the full value when clicked.
 */
class NoteButton: public QToolButton
{
    Q_OBJECT
public:
    NoteButton(const QString &colName, QWidget *parent = 0);

    QString content();
    void setContent(const QString &text);

protected:
    void showEvent(QShowEvent *event);

private slots:
    void launchEditor();

private:
    QString name; /**< The name of the column the note belongs to */
    QString noteContent; /**< The current note text */
};

#endif
