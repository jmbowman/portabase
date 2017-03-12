/*
 * qqfactory.cpp
 *
 * (c) 2008-2012,2016-2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file qqutil/qqfactory.cpp
 * Source file for QQFactory
 */

#include <QAbstractItemView>
#include <QAbstractScrollArea>
#include <QApplication>
#include <QComboBox>
#include <QListView>
#include <QLocale>
#include <QProcess>
#include <QPushButton>
#include <QToolButton>
#include <QTranslator>
#include "qqfactory.h"

#if QT_VERSION >= 0x050000
#include <QScreen>
#include <QScroller>
#endif

/**
 * Create a QGridLayout and configure it with no margin or spacing (to better
 * take advantage of available space on small screens, primarily).
 *
 * @param parent The new layout's parent widget
 * @param useForParent True if the result should be set as the parent's layout
 * @return The new layout
 */
QGridLayout *QQFactory::gridLayout(QWidget *parent, bool useForParent)
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
QGridLayout *QQFactory::gridLayout(QBoxLayout *parent)
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
QHBoxLayout *QQFactory::hBoxLayout(QWidget *parent, bool useForParent)
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
QHBoxLayout *QQFactory::hBoxLayout(QBoxLayout *parent)
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
QVBoxLayout *QQFactory::vBoxLayout(QWidget *parent, bool useForParent)
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
QVBoxLayout *QQFactory::vBoxLayout(QBoxLayout *parent)
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
void QQFactory::setupLayout(QLayout *layout)
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
 * Get the best kind of button to use for the current platform.  This is
 * typically QToolButton on the Mac and QPushButton elsewhere (since Mac
 * push buttons generally don't use icons).
 *
 * @param parent The parent widget of the button to be created
 * @return The new button
 */
QAbstractButton *QQFactory::button(QWidget *parent)
{
#if defined(Q_OS_MAC)
    return new QToolButton(parent);
#else
    return new QPushButton(parent);
#endif
}

/**
 * Get a properly-configured QComboBox for the current platform.  Makes the
 * popup items finger-sized on Android.
 *
 * @param parent The parent widget of the combo box to be created
 * @return The new combo box
 */
QComboBox *QQFactory::comboBox(QWidget *parent)
{
    QComboBox *cb = new QComboBox(parent);
#if defined(Q_OS_ANDROID)
    cb->setStyleSheet(QStringLiteral("QComboBox QAbstractItemView::item {min-height: %1px}").arg(dpToPixels(48)));
    QListView *popup = new QListView(cb);
    cb->setView(popup);
    // For some reason, QScroller only works with combo boxes on left mouse
    // gestures, not touch gestures.  The reverse is true for some other
    // widgets, so can't use configureAbstractItemView() here
    popup->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    popup->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    popup->setMouseTracking(false);
    popup->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    popup->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QScroller::grabGesture(popup, QScroller::LeftMouseButtonGesture);
#endif
    return cb;
}

/**
 * Get a properly-configured QTextEdit for the current platform.  Enables
 * text selection on Android.
 *
 * @param parent The parent widget of the text editor to be created
 * @return The new text editor
 */
QTextEdit *QQFactory::textEdit(QWidget *parent)
{
    QTextEdit *editor = new QTextEdit(parent);
#if defined(Q_OS_ANDROID)
    editor->setTextInteractionFlags(editor->textInteractionFlags() | Qt::TextSelectableByMouse);
#endif
    return editor;
}

/**
 * Convert the provided dimension in dp (device-independent pixels) to the
 * corresponding number of actual pixels on the current display.  Currently
 * only used on Android.
 *
 * @param dp The value in dp to be converted
 * @return The corresponding number of actual pixels
 */
int QQFactory::dpToPixels(int dp)
{
#if QT_VERSION >= 0x050000
    return (dp * qApp->primaryScreen()->physicalDotsPerInch()) / 160;
#else
    return dp;
#endif
}

/**
 * Load the icon of the specified name in whatever format and manner is
 * appropriate for the current platform.
 *
 * @param name The base name of the icon's alias in the QRC file
 * @return The loaded icon
 */
QIcon QQFactory::icon(const QString &name)
{
#ifdef Q_OS_ANDROID
    return QIcon(QString(":/icons/%1.svg").arg(name));
#else
    return QIcon(QString(":/icons/%1.png").arg(name));
#endif
}

/**
 * Configure the given item view to use the appropriate scrolling mechanism
 * for the current platform.  On Android this disables the scrollbars,
 * enables kinetic touch scrolling, and uses pixel-based scrolling.
 *
 * @param view
 */
void QQFactory::configureAbstractItemView(QAbstractItemView *view)
{
    view->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    view->setMouseTracking(false);
    configureScrollArea(view);
}

/**
 * Configure the given scrollable widget to use the appropriate scrolling
 * mechanism for the current platform.  On Android, disables the scrollbars
 * and enables kinetic touch scrolling.
 *
 * @param scrollArea The scrollable widget to be configured
 */
void QQFactory::configureScrollArea(QAbstractScrollArea *scrollArea)
{
#if defined(Q_OS_ANDROID)
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QScroller::grabGesture(scrollArea, QScroller::TouchGesture);
#else
    Q_UNUSED(scrollArea)
#endif
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
void QQFactory::translation(QTranslator *translator, const QString &filename,
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
