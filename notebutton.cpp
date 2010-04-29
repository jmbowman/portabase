/*
 * notebutton.cpp
 *
 * (c) 2002-2003,2009-2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file notebutton.cpp
 * Source file for NoteButton
 */

#include <QFontMetrics>
#include <QRegExp>
#include "notebutton.h"
#include "noteeditor.h"
#include "portabase.h"

/**
 * Constructor.
 *
 * @param colName The name of the column this note belongs to
 * @param parent This button's parent widget
 */
NoteButton::NoteButton(const QString &colName, QWidget *parent)
#if defined(Q_WS_MAEMO_5)
    : QPushButton(parent), name(colName)
#else
    : QToolButton(parent), name(colName)
#endif
{
    setIcon(QIcon(":/icons/note.png"));
#if !defined(Q_WS_MAEMO_5)
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,
                              QSizePolicy::Fixed, QSizePolicy::ToolButton));
#endif
    connect(this, SIGNAL(clicked()), this, SLOT(launchEditor()));
}

/**
 * Get the note's current content.
 *
 * @return The note text
 */
QString NoteButton::content()
{
    return noteContent;
}

/**
 * Set the note's content.
 *
 * @param text The note's new text
 */
void NoteButton::setContent(const QString &text)
{
    noteContent = text;
    QString buttonText(text);
    buttonText.replace(QRegExp("\n"), " ");
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

/**
 * Launch the note content editor dialog.  Launched when the button is
 * clicked.
 */
void NoteButton::launchEditor()
{
    NoteEditor editor(name, false, this);
    editor.setContent(noteContent);
    if (editor.exec()) {
        setContent(editor.content());
    }
}

/**
 * Adjust the button's text accordingly when the button is resized.
 *
 * @param event A resize event for the button
 */
void NoteButton::resizeEvent(QResizeEvent *event)
{
  QAbstractButton::resizeEvent(event);
  setContent(noteContent);
}

/**
 * Set the button's text to the appropriate subset of the note content
 * before it is shown.
 *
 * @param event A widget show event for the button
 */
void NoteButton::showEvent(QShowEvent *event)
{
    QAbstractButton::showEvent(event);
    setContent(noteContent);
}
