/*
 * factory.h
 *
 * (c) 2008-2012 by Jeremy Bowman <jmbowman@alum.mit.edu>
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

#include <QIcon>
#include <QLayout>
#include <QStringList>

class QAbstractButton;
class QAbstractItemView;
class QListWidget;
class QSettings;
class QTranslator;
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
class Factory
{
public:
    static QGridLayout *gridLayout(QWidget *parent, bool useForParent=false);
    static QGridLayout *gridLayout(QBoxLayout *parent);
    static QHBoxLayout *hBoxLayout(QWidget *parent, bool useForParent=false);
    static QHBoxLayout *hBoxLayout(QBoxLayout *parent);
    static QVBoxLayout *vBoxLayout(QWidget *parent, bool useForParent=false);
    static QVBoxLayout *vBoxLayout(QBoxLayout *parent);
    static QListWidget *listWidget(QWidget *parent);
    static QTreeWidget *treeWidget(QWidget *parent, const QStringList &headers);
    static QAbstractButton *button(QWidget *parent);
    static HtmlDisplay *htmlDisplay(QWidget *parent);
    static void translation(QTranslator *translator, const QString &filename,
                            const QString &envVariable);
    static void updatePreferences(QSettings *settings);
    static void updateRowColors(QAbstractItemView *view);

private:
    static void setupLayout(QLayout *layout);

public:
    static QColor evenRowColor;
    static QColor oddRowColor;

private:
    static bool useAlternatingRowColors;
    static bool webViewConfigured;
};

#endif
