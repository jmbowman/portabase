/*
 * dynamicedit.cpp
 *
 * (c) 2003,2008-2010,2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file dynamicedit.cpp
 * Source file for DynamicEdit
 */

#include <QApplication>
#include <QFontMetrics>
#include "dynamicedit.h"
#include "factory.h"

/**
 * Constructor.
 *
 * @param parent This widget's parent widget, if any
 */
DynamicEdit::DynamicEdit(QWidget *parent)
  : QTextEdit(parent), sampleDoc(0)
{
    sampleDoc = new QTextDocument("Ag", this);
    setTabChangesFocus(true);
    setFixedHeight(sizeHint().height());
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(this, SIGNAL(textChanged()), this, SLOT(adjustHeight()));
    setAcceptRichText(false);
    setLineWrapMode(QTextEdit::NoWrap);
    setWordWrapMode(QTextOption::NoWrap);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    Factory::configureScrollArea(this);
#if defined(Q_OS_ANDROID)
    int pixels = Factory::dpToPixels(6);
    setStyleSheet(QString("QTextEdit {padding-left:%1px; padding-top:%1px; padding-bottom:%1px; padding-right:%1px}").arg(pixels));
    setTextInteractionFlags(textInteractionFlags() | Qt::TextSelectableByMouse);
#endif
}

/**
 * Determine the preferred size of this widget based on its current text
 * content.
 *
 * @return The current preferred size
 */
QSize DynamicEdit::sizeHint() const
{
    QString content = toPlainText();
    QTextDocument *doc = document();
    int height;
    if (content.isEmpty()) {
        doc = sampleDoc;
    }
    qreal rheight = doc->size().rheight();
    height = qRound(rheight);
    // Want to round up, not down
    if (rheight > height) {
        height++;
    }
#if defined(Q_WS_MAEMO_5)
    // on this platform, need to manually add room for the padding, etc.
    height += 30;
#elif defined(Q_OS_ANDROID)
    height += Factory::dpToPixels(12);
#endif
    return QSize(100, height);
}

/**
 * Recalculate the height required to contain all the lines of text entered
 * so far.  Called whenever the content of the text field changes.
 */
void DynamicEdit::adjustHeight()
{
    setFixedHeight(sizeHint().height());
    updateGeometry();
}
