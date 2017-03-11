/*
 * qqandroidstyle.h
 *
 * (c) 2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file qqutil/qqandroidstyle.h
 * Header file for QQAndroidStyle
 */

#include <QString>
#ifdef Q_OS_ANDROID
#ifndef QQ_ANDROID_STYLE_H
#define QQ_ANDROID_STYLE_H

#include <QProxyStyle>

/**
 * Style overrides for Qt Widgets applications on Android.
 *
 * * Assigns a finger-friendly height to list and tree view items
 * * Gives some left padding to QAbstractItemView and QHeaderView cells
 * * Adds a little more horizontal padding to QPushButtons, since they tend
 *   to get cropped on high resolution screens
 * * Always draws QToolButtons in a flat style (no elevation or highlighting)
 * * Doesn't show focus highlighting on QPushButtons
 * * Doesn't show shortcut underlines in QPushButton text
 * * Uses more appropriate SVG choices for some of the standard icons
 * * Set appropriate sizes for icons in menus
 */
class QQAndroidStyle: public QProxyStyle
{
public:
    QQAndroidStyle(QStyle *style = Q_NULLPTR);

    void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option,
                            QPainter *painter, const QWidget *widget = Q_NULLPTR) const;
    void drawControl(ControlElement element, const QStyleOption *option,
                     QPainter *painter, const QWidget *widget = Q_NULLPTR) const;
    void drawItemText(QPainter *painter, const QRect &rectangle, int alignment,
                      const QPalette &palette, bool enabled, const QString &text,
                      QPalette::ColorRole textRole = QPalette::NoRole) const;
    int pixelMetric(PixelMetric metric, const QStyleOption* option = 0,
                    const QWidget* widget = 0) const;
    QSize sizeFromContents(ContentsType type, const QStyleOption *option,
                           const QSize &size, const QWidget *widget) const;
    QIcon standardIcon(StandardPixmap standardIcon,
                       const QStyleOption *option = Q_NULLPTR,
                       const QWidget *widget = Q_NULLPTR) const;
    QRect subElementRect(SubElement element, const QStyleOption *option,
                         const QWidget *widget) const;
};
#endif // QQ_ANDROID_STYLE_H
#endif // Q_OS_ANDROID
