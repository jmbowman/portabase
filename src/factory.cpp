/*
 * factory.cpp
 *
 * (c) 2008-2012,2016-2017,2020 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file factory.cpp
 * Source file for Factory
 */

#include <QApplication>
#include <QHeaderView>
#include <QListWidget>
#include <QPalette>
#include <QSettings>
#include <QTreeWidget>
#include "factory.h"

#if defined(Q_WS_MAEMO_5)
#include <QWebSettings>
#include <QWebView>
#endif

QColor Factory::evenRowColor(Qt::white);
QColor Factory::oddRowColor(Qt::lightGray);
bool Factory::useAlternatingRowColors(true);
bool Factory::webViewConfigured(false);

/**
 * Reload the application settings for row color options.  Called when the
 * application is first started, and again whenever changes are made in the
 * Preferences dialog.
 */
void Factory::updatePreferences(QSettings *settings)
{
    settings->beginGroup("Colors");
    useAlternatingRowColors = settings->value("UseAlternating",
                                              true).toBool();
    if (settings->contains("EvenRows")) {
        evenRowColor = QColor(settings->value("EvenRows").toString());
        evenRowColor.setAlpha(settings->value("EvenRowsAlpha", 255).toInt());
    }
    else {
        evenRowColor = qApp->palette("QAbstractItemView").color(QPalette::Base);
    }
    if (settings->contains("OddRows")) {
        oddRowColor = QColor(settings->value("OddRows").toString());
        oddRowColor.setAlpha(settings->value("OddRowsAlpha", 255).toInt());
    }
    else {
#if defined(Q_OS_ANDROID)
        // The theme default alternate color is way too close to white
        oddRowColor = QColor("lightblue");
#else
        oddRowColor = qApp->palette("QAbstractItemView").color(QPalette::AlternateBase);
#endif
    }
    settings->endGroup();
}

/**
 * Create and configure a QListWidget.
 *
 * @param parent The parent widget of the widget to be created
 * @return The new list widget
 */
QListWidget *Factory::listWidget(QWidget *parent)
{
    QListWidget *list = new QListWidget(parent);
    list->setSelectionMode(QAbstractItemView::SingleSelection);
    updateRowColors(list);
    configureAbstractItemView(list);
#if defined(Q_OS_ANDROID)
    list->setStyleSheet(QStringLiteral("QListView {font-size: 16pt;}"));
#endif
    return list;
}

/**
 * Create and configure a QTreeWidget.
 *
 * @param parent The parent widget of the widget to be created
 * @param headers The header labels to use for the tree widget
 * @return The new tree widget
 */
QTreeWidget *Factory::treeWidget(QWidget *parent, const QStringList &headers)
{
    QTreeWidget *table = new QTreeWidget(parent);
    QHeaderView *header = table->header();
    table->setUniformRowHeights(true);
    int colCount = headers.count();
    if (colCount > 0) {
        table->setColumnCount(headers.count());
        table->setHeaderLabels(headers);
#if QT_VERSION >= 0x050000
        header->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
        header->setResizeMode(QHeaderView::ResizeToContents);
#endif
    }
#if QT_VERSION >= 0x050000
    header->setSectionsMovable(false);
#else
    header->setMovable(false);
#endif
    table->setSortingEnabled(false);
    table->setAllColumnsShowFocus(true);
    table->setRootIsDecorated(false);
    updateRowColors(table);
    configureAbstractItemView(table);
#if defined(Q_OS_ANDROID)
    int headerHeight = dpToPixels(48);
    header->setStyleSheet(QStringLiteral("QHeaderView::section {height: %1px;}").arg(headerHeight));
#endif
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    return table;
}

/**
 * Create and configure a read-only QTextEdit (for showing simple HTML
 * content).  On Maemo Fremantle, a QWebView will be used instead to work
 * around an intermittent crash-inducing bug in adding HTML to QTextEdit.
 *
 * @param parent The parent widget of the widget to be created
 * @return The new text display widget
 */
HtmlDisplay *Factory::htmlDisplay(QWidget *parent)
{
#if defined(Q_WS_MAEMO_5)
    if (!webViewConfigured) {
        QWebSettings *settings = QWebSettings::globalSettings();
        settings->setAttribute(QWebSettings::JavascriptEnabled, false);
        settings->setAttribute(QWebSettings::PrivateBrowsingEnabled, true);
        settings->setAttribute(QWebSettings::SiteSpecificQuirksEnabled, false);
        settings->setFontSize(QWebSettings::DefaultFontSize, qApp->font().pointSize());
        settings->setMaximumPagesInCache(1);
        webViewConfigured = true;
    }
#endif
    HtmlDisplay *display = new HtmlDisplay(parent);
#if defined(Q_OS_ANDROID)
    display->setTextInteractionFlags(display->textInteractionFlags() | Qt::TextSelectableByMouse);
#endif
#if !defined(Q_WS_MAEMO_5)
    display->setReadOnly(true);
    configureScrollArea(display);
#endif
    return display;
}

/**
 * Update the row colors for a list or tree widget to match the ones currently
 * specified in the application preferences.
 *
 * @param view The list or tree widget to be modified
 */
void Factory::updateRowColors(QAbstractItemView *view)
{
    // Maemo 5 uses the GTK+ theme colors, can't override from here; just
    // screws up the background color
    view->setAlternatingRowColors(useAlternatingRowColors);
#if !defined(Q_WS_MAEMO_5)
    QPalette viewPalette(view->palette());
    viewPalette.setColor(QPalette::Base, evenRowColor);
    viewPalette.setColor(QPalette::AlternateBase, oddRowColor);
    view->setPalette(viewPalette);
#endif
}
