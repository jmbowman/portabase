/*
 * qqfactory.h
 *
 * (c) 2008-2012,2016-2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file qqutil/qqfactory.h
 * Header file for QQFactory
 */

#ifndef QQFACTORY_H
#define QQFACTORY_H

#include <QIcon>
#include <QLayout>
#include <QLineEdit>
#include <QStringList>

class QAbstractButton;
class QAbstractItemView;
class QAbstractScrollArea;
class QComboBox;
class QTranslator;
class QWidget;

#if defined(Q_WS_MAEMO_5)
#include <QWebView>
#define HtmlDisplay QWebView
class QTextEdit;
#else
#include <QTextEdit>
#define HtmlDisplay QTextEdit
#endif

/**
 * A collection of static factory methods for creating widgets and layouts
 * which have relatively complex default configurations.
 */
class QQFactory
{
public:
    static QGridLayout *gridLayout(QWidget *parent, bool useForParent = false);
    static QGridLayout *gridLayout(QBoxLayout *parent);
    static QHBoxLayout *hBoxLayout(QWidget *parent, bool useForParent = false);
    static QHBoxLayout *hBoxLayout(QBoxLayout *parent);
    static QVBoxLayout *vBoxLayout(QWidget *parent, bool useForParent = false);
    static QVBoxLayout *vBoxLayout(QBoxLayout *parent);
    static QAbstractButton *button(QWidget *parent);
    static QComboBox *comboBox(QWidget *parent);
    static QTextEdit *textEdit(QWidget *parent);
    static int dpToPixels(int dp);
    static QIcon icon(const QString &name);
    static void translation(QTranslator *translator, const QString &filename,
                            const QString &envVariable);
    static void configureAbstractItemView(QAbstractItemView *view);
    static void configureScrollArea(QAbstractScrollArea *scrollArea);

private:
    static void setupLayout(QLayout *layout);
};

#endif
