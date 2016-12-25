/*
 * factory.cpp
 *
 * (c) 2008-2012,2016 by Jeremy Bowman <jmbowman@alum.mit.edu>
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
#include <QLocale>
#include <QPainter>
#include <QPalette>
#include <QProcess>
#include <QPushButton>
#include <QSettings>
#include <QToolButton>
#include <QTranslator>
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
    QString color = settings->value("EvenRows", "#FFFFFF").toString();
    evenRowColor = QColor(color);
    color = settings->value("OddRows", "#E0E0E0").toString();
    oddRowColor = QColor(color);
    settings->endGroup();
}

/**
 * Create a QGridLayout and configure it with no margin or spacing (to better
 * take advantage of available space on small screens, primarily).
 *
 * @param parent The new layout's parent widget
 * @param useForParent True if the result should be set as the parent's layout
 * @return The new layout
 */
QGridLayout *Factory::gridLayout(QWidget *parent, bool useForParent)
{
    QGridLayout *grid = new QGridLayout(parent);
    setupLayout(grid);
    if (useForParent) {
        parent->setLayout(grid);
    }
    return grid;
}

/**
 * Create a QGridLayout and configure it with no margin or spacing (to better
 * take advantage of available space on small screens, primarily).
 *
 * @param parent The new layout's parent layout
 * @return The new layout
 */
QGridLayout *Factory::gridLayout(QBoxLayout *parent)
{
    QGridLayout *grid = new QGridLayout();
    parent->addLayout(grid);
    setupLayout(grid);
    return grid;
}

/**
 * Create a QHBoxLayout and configure it with no margin or spacing (to better
 * take advantage of available space on small screens, primarily).
 *
 * @param parent The new layout's parent widget
 * @param useForParent True if the result should be set as the parent's layout
 * @return The new layout
 */
QHBoxLayout *Factory::hBoxLayout(QWidget *parent, bool useForParent)
{
    QHBoxLayout *hbox = new QHBoxLayout(parent);
    setupLayout(hbox);
    if (useForParent) {
        parent->setLayout(hbox);
    }
    return hbox;
}

/**
 * Create a QHBoxLayout and configure it with no margin or spacing (to better
 * take advantage of available space on small screens, primarily).
 *
 * @param parent The new layout's parent layout
 * @return The new layout
 */
QHBoxLayout *Factory::hBoxLayout(QBoxLayout *parent)
{
    QHBoxLayout *hbox = new QHBoxLayout();
    parent->addLayout(hbox);
    setupLayout(hbox);
    return hbox;
}

/**
 * Create a QVBoxLayout and configure it with no margin or spacing (to better
 * take advantage of available space on small screens, primarily).
 *
 * @param parent The new layout's parent widget
 * @param useForParent True if the result should be set as the parent's layout
 * @return The new layout
 */
QVBoxLayout *Factory::vBoxLayout(QWidget *parent, bool useForParent)
{
    QVBoxLayout *vbox = new QVBoxLayout(parent);
    setupLayout(vbox);
    if (useForParent) {
        parent->setLayout(vbox);
    }
    return vbox;
}

/**
 * Create a QVBoxLayout and configure it with no margin or spacing (to better
 * take advantage of available space on small screens, primarily).
 *
 * @param parent The new layout's parent layout
 * @return The new layout
 */
QVBoxLayout *Factory::vBoxLayout(QBoxLayout *parent)
{
    QVBoxLayout *vbox = new QVBoxLayout();
    parent->addLayout(vbox);
    setupLayout(vbox);
    return vbox;
}

/**
 * Setup code common for all new layouts.
 *
 * @param layout The layout being configured
 */
void Factory::setupLayout(QLayout *layout)
{
    // It's important to keep margins inside QGroupBoxes, or they look wrong
    if (!layout->parentWidget()->inherits("QGroupBox")) {
        layout->setContentsMargins(0, 0, 0, 0);
    }
#if defined(Q_WS_MAEMO_5)
    else {
        // default top margin on Fremantle is insufficient
        QMargins margins = layout->contentsMargins();
        margins.setTop(20);
        layout->setContentsMargins(margins);
    }
#endif
    layout->setSpacing(0);
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
    updateRowColors(list);
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
    table->setUniformRowHeights(true);
    int colCount = headers.count();
    if (colCount > 0) {
        table->setColumnCount(headers.count());
        table->setHeaderLabels(headers);
#if QT_VERSION >= 0x050000
        table->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
        table->header()->setResizeMode(QHeaderView::ResizeToContents);
#endif
    }
#if QT_VERSION >= 0x050000
    table->header()->setSectionsMovable(false);
#else
    table->header()->setMovable(false);
#endif
    table->setSortingEnabled(false);
    table->setAllColumnsShowFocus(true);
    table->setRootIsDecorated(false);
    updateRowColors(table);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    return table;
}

/**
 * Get the best kind of button to use for the current platform.  This is
 * typically QToolButton on the Mac and QPushButton elsewhere (since Mac
 * push buttons generally don't use icons).
 *
 * @param parent The parent widget of the button to be created
 * @return The new button
 */
QAbstractButton *Factory::button(QWidget *parent)
{
#if defined(Q_OS_MAC)
    return new QToolButton(parent);
#else
    return new QPushButton(parent);
#endif
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
#if !defined(Q_WS_MAEMO_5)
    display->setReadOnly(true);
#endif
    return display;
}

/**
 * Load a UI translation file from a resource based on the appropriate system
 * UI language settings.  Assumes that such files are in the standard ":/i18n"
 * folder, and that an underscore is used to separate the base filename from
 * the language.  Allows for overriding this selection by setting an
 * appropriate environment variable to the .qm file to use instead (useful for
 * translators to test their work).
 *
 * @param translator The translator object to be loaded and installed
 * @param filename The base filename of the translation file, before appending
 *                 the language
 * @param envVariable The name of the override environment variable
 */
void Factory::translation(QTranslator *translator, const QString &filename,
                          const QString &envVariable)
{
    // can't count on QProcessEnvironment because Diablo doesn't have it yet
    QStringList env = QProcess::systemEnvironment();
    int index = env.indexOf(QRegExp(envVariable + "=.*"));
    if (index != -1) {
        QString path = env[index];
        path = path.right(path.length() - envVariable.length() - 1);
        if (translator->load(path)) {
            qApp->installTranslator(translator);
        }
    }
    else {
#if QT_VERSION >= 0x040800
        // Use the correct list of UI languages from the system locale
        bool loaded = translator->load(QLocale::system(), filename, "_",
                                       ":/i18n");
#else
        // Hope that Qt isn't returning a formatting locale different from the
        // UI language one
        bool loaded = translator->load(QString(":/i18n/") + filename + ".qm");
#endif
        if (loaded) {
            qApp->installTranslator(translator);
        }
    }
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
