/*
 * factory.cpp
 *
 * (c) 2008-2011 by Jeremy Bowman <jmbowman@alum.mit.edu>
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
#include <QPainter>
#include <QPalette>
#include <QPushButton>
#include <QSettings>
#include <QTextEdit>
#include <QToolButton>
#include <QTreeWidget>
#include "factory.h"

#if defined(Q_WS_MAEMO_5)
#include <QAbstractKineticScroller>
#endif

QColor Factory::evenRowColor(Qt::white);
QColor Factory::oddRowColor(Qt::lightGray);
bool Factory::useAlternatingRowColors(true);

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
      table->header()->setResizeMode(QHeaderView::ResizeToContents);
    }
    table->header()->setMovable(false);
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
#if defined(Q_WS_MAC)
    return new QToolButton(parent);
#else
    return new QPushButton(parent);
#endif
}

/**
 * Create and configure a read-only QTextEdit (for showing simple HTML
 * content).  On Maemo Fremantle, kinetic scrolling will be enabled.
 *
 * @param parent The parent widget of the widget to be created
 * @return The new text display widget
 */
QTextEdit *Factory::textDisplay(QWidget *parent)
{
    QTextEdit *display = new QTextEdit(parent);
    display->setReadOnly(true);
#if defined(Q_WS_MAEMO_5)
    QVariant ksProp = display->property("kineticScroller");
    QAbstractKineticScroller *ks = ksProp.value<QAbstractKineticScroller *>();
    if (ks) {
        ks->setEnabled(true);
    }
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
