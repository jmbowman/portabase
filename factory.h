/*
 * factory.h
 *
 * (c) 2008-2009 by Jeremy Bowman <jmbowman@alum.mit.edu>
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

class QAbstractItemView;
class QGridLayout;
class QHBoxLayout;
class QListWidget;
class QTreeWidget;
class QVBoxLayout;
class QWidget;

/**
 * A collection of static factory methods for creating widgets and layouts
 * which have relatively complex default configurations.
 */
class Factory
{
public:
    static QGridLayout *gridLayout(QWidget *parent, bool useForParent=false);
    static QHBoxLayout *hBoxLayout(QWidget *parent, bool useForParent=false);
    static QVBoxLayout *vBoxLayout(QWidget *parent, bool useForParent=false);
    static QListWidget *listWidget(QWidget *parent);
    static QTreeWidget *treeWidget(QWidget *parent, const QStringList &headers);
    static QIcon checkBoxIcon(int checked);
    static void updateRowColors(QAbstractItemView *view);
    static void createCheckBoxIcons();

public:
    static QColor evenRowColor;
    static QColor oddRowColor;
    static QIcon checkedIcon;
    static QIcon uncheckedIcon;
};

#endif