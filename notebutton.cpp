/*
 * notebutton.cpp
 *
 * (c) 2002-2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qfontmetrics.h>
#include <qregexp.h>
#include "notebutton.h"
#include "noteeditor.h"
#include "portabase.h"

NoteButton::NoteButton(QString colName, QWidget *parent, const char *name)
    : QPushButton(PortaBase::getNotePixmap(), "", parent, name), name(colName)
{
    connect(this, SIGNAL(clicked()), this, SLOT(launchEditor()));
}

NoteButton::~NoteButton()
{

}

QString NoteButton::content()
{
    return noteContent;
}

void NoteButton::setContent(QString text)
{
    noteContent = text;
    QString buttonText = text.replace(QRegExp("\n"), " ");
    QFontMetrics metrics = fontMetrics();
    int available = width() - 60 - metrics.width("...");
    int length = buttonText.length();
    for (int i = 0; i < length; i++) {
        if (metrics.width(buttonText, i + 1) > available) {
            buttonText = buttonText.left(i) + "...";
            break;
        }
    }
    buttonText = buttonText.replace(QRegExp("&"), "&&");
    setText(buttonText);
}

void NoteButton::launchEditor()
{
    NoteEditor editor(name, FALSE, this);
    editor.setContent(noteContent);
    if (editor.exec()) {
        setContent(editor.content());
    }
}

void NoteButton::showEvent(QShowEvent *event)
{
    QPushButton::showEvent(event);
    setContent(noteContent);
}
