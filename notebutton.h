/*
 * notebutton.h
 *
 * (c) 2002 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef NOTEBUTTON_H
#define NOTEBUTTON_H

#include <qpushbutton.h>

class NoteButton: public QPushButton
{
    Q_OBJECT
public:
    NoteButton(QString colName, QWidget *parent = 0, const char *name = 0);
    ~NoteButton();

    QString content();
    void setContent(QString text);

protected:
    void showEvent(QShowEvent *event);

private slots:
    void launchEditor();

private:
    QString name;
    QString noteContent;
};

#endif
