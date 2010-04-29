/*
 * notebutton.h
 *
 * (c) 2002,2009-2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
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

#include <QPushButton>
#include <QToolButton>

/**
 * Button which represents a note field.  Displays the beginning of the note
 * text, and launches an editor for the full value when clicked.
 */
#if defined(Q_WS_MAEMO_5)
class NoteButton: public QPushButton
#else
class NoteButton: public QToolButton
#endif
{
    Q_OBJECT
public:
    NoteButton(const QString &colName, QWidget *parent = 0);

    QString content();
    void setContent(const QString &text);

protected:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);

private slots:
    void launchEditor();

private:
    QString name; /**< The name of the column the note belongs to */
    QString noteContent; /**< The current note text */
};

#endif
