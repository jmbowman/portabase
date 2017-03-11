/*
 * factory.h
 *
 * (c) 2008-2012,2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file factory.h
 * Header file for Factory
 */

#ifndef FACTORY_H
#define FACTORY_H

#include <QStringList>
#include "qqutil/qqfactory.h"

class QListWidget;
class QSettings;
class QTreeWidget;
class QWidget;

#if defined(Q_WS_MAEMO_5)
#include <QWebView>
#define HtmlDisplay QWebView
#else
#include <QTextEdit>
#define HtmlDisplay QTextEdit
#endif

/**
 * A collection of static factory methods for creating widgets and layouts
 * which have relatively complex default configurations.
 */
class Factory : public QQFactory
{
public:
    static QListWidget *listWidget(QWidget *parent);
    static QTreeWidget *treeWidget(QWidget *parent, const QStringList &headers);
    static HtmlDisplay *htmlDisplay(QWidget *parent);
    static void updatePreferences(QSettings *settings);
    static void updateRowColors(QAbstractItemView *view);

public:
    static QColor evenRowColor;
    static QColor oddRowColor;

private:
    static bool useAlternatingRowColors;
    static bool webViewConfigured;
};

#endif
