/*
 * qqandroidstyle.cpp
 *
 * (c) 2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file qqutil/qqandroidstyle.cpp
 * Source file for QQAndroidStyle
 */

#include <QPainter>
#include <QPushButton>
#include <QStyleOptionToolButton>
#include <QWidget>
#include "qqandroidstyle.h"
#include "qqfactory.h"

#if defined(Q_OS_ANDROID)

/**
 * Constructor.
 */
QQAndroidStyle::QQAndroidStyle(QStyle *style) : QProxyStyle(style)
{

}

/**
 * Draws the given control using the provided painter with the given style
 * options.  Overridden so that QToolButtons are always drawn in the default
 * flat style.
 *
 * @param control The type of control to draw
 * @param option The style options for the control
 * @param painter The painer to draw the control with
 * @param widget The widget being drawn
 */
void QQAndroidStyle::drawComplexControl(
        ComplexControl control, const QStyleOptionComplex *option,
        QPainter *painter, const QWidget *widget) const
{
    if (control == CC_ToolButton) {
        const QStyleOptionToolButton *opt = reinterpret_cast<const QStyleOptionToolButton *>(option);
        QStyleOptionToolButton newOption(*opt);
        newOption.state = State_AutoRaise;
        if (option->state & State_Enabled) {
            newOption.state |= State_Enabled;
        }
        QProxyStyle::drawComplexControl(control, &newOption, painter, widget);
        return;
    }
    QProxyStyle::drawComplexControl(control, option, painter, widget);
}

/**
 * Draws the given element with the provided painter with the style options
 * specified by option.  Overridden in order to avoid showing buttons with
 * input focus as highlighted.
 *
 * @param element The type of control to draw
 * @param option The style options for the control
 * @param painter The painter to draw the control with
 * @param widget The widget being drawn
 */
void QQAndroidStyle::drawControl(
        ControlElement element, const QStyleOption *option,
        QPainter *painter, const QWidget *widget) const
{
    if (element == CE_PushButton) {
        if (const QStyleOptionButton *opt = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            if (opt->state | State_HasFocus) {
                QStyleOptionButton newOption(*opt);
                newOption.state &= ~State_HasFocus;
                QProxyStyle::drawControl(element, &newOption, painter, widget);
                return;
            }
        }
    }
    QProxyStyle::drawControl(element, option, painter, widget);
}

/**
 * Draws the given text in the specified rectangle using the provided painter
 * and palette.  Overridden in order to surpress shortcut underlines on push
 * buttons.
 *
 * @param painter The painter to draw with
 * @param rectangle The bounding rectangle of the text
 * @param alignment The alignment flags to use
 * @param palette The color palette to draw with
 * @param enabled True if the text is in an enabled widget
 * @param text The text to draw
 * @param textRole The role of this text in the UI
 */
void QQAndroidStyle::drawItemText(QPainter *painter, const QRect &rectangle,
                                  int alignment, const QPalette &palette,
                                  bool enabled, const QString &text,
                                  QPalette::ColorRole textRole) const
{
    int newAlignment = alignment;
    if (alignment & Qt::TextShowMnemonic) {
        newAlignment &= ~Qt::TextShowMnemonic;
        newAlignment |= Qt::TextHideMnemonic;
    }
    return QProxyStyle::drawItemText(painter, rectangle, newAlignment,
                                     palette, enabled, text, textRole);
}

/**
 * Returns the value of the given pixel metric.  Overridden in order to set
 * appropriate icon sizes in buttons, item views, and menus.
 *
 * @param metric The type of metric to obtain
 * @param option Any extra information needed to get the correct value
 * @param widget The widget being rendered, if any
 * @return A number of pixels
 */
int QQAndroidStyle::pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const {
    switch (metric) {
    case PM_ButtonIconSize:
    case PM_ListViewIconSize:
    case PM_SmallIconSize:
    case PM_ToolBarIconSize:
        return QQFactory::dpToPixels(24);
    case PM_IconViewIconSize:
        return QQFactory::dpToPixels(36);
    case PM_LargeIconSize:
        return QQFactory::dpToPixels(48);
    default:
        return QProxyStyle::pixelMetric(metric, option, widget);
    }
}

/**
 * Returns the size of the element described by the specified option and
 * type, based on the provided contentsSize.  Overridden in order to use
 * finger-friendly heights for item views and avoid text overflow in
 * push buttons.
 *
 * @param type The type of element being sized
 * @param option A pointer to a QStyleOption or one of its subclasses
 * @param size Optionally pre-calculated content size
 * @param widget The widget in question, which can contain extra information
 *               used for calculating the size
 * @return The suggested size for the element
 */
QSize QQAndroidStyle::sizeFromContents(ContentsType type, const QStyleOption *option,
                                  const QSize &size, const QWidget *widget) const
{
    QSize sz = QProxyStyle::sizeFromContents(type, option, size, widget);
    if (type == CT_PushButton) {
        const QPushButton *button = qobject_cast<const QPushButton *>(widget);
        if (!button->icon().isNull() && !button->text().isEmpty()) {
            sz.rwidth() += QQFactory::dpToPixels(25);
        }
    }
    else if (type == CT_ItemViewItem) {
        sz.setHeight(QQFactory::dpToPixels(48));
        // Don't cut off the right edge of the text
        sz.setWidth(sz.width() + QQFactory::dpToPixels(25));
    }
    return sz;
}

/**
 * Returns an icon for the given standardIcon.  Overridden in order to show
 * appropriately-sized icons in file dialogs.
 *
 * @param standardIcon The type of icon to retrieve
 * @param option Any extra information needed to choose the correct icon
 * @param widget The widget in which the icon will be used
 * @return The icon to be displayed
 */
QIcon QQAndroidStyle::standardIcon(
        StandardPixmap standardIcon, const QStyleOption *option,
        const QWidget *widget) const
{
    switch (standardIcon) {
    case SP_ArrowLeft:
        return QQFactory::icon(QStringLiteral("back"));
    case SP_ArrowRight:
        return QQFactory::icon(QStringLiteral("forward"));
    case SP_ComputerIcon:
        return QQFactory::icon(QStringLiteral("computer"));
    case SP_DirIcon:
    case SP_DirHomeIcon:
    case SP_DirLinkIcon:
    case SP_DriveHDIcon:
        return QQFactory::icon(QStringLiteral("folder"));
    case SP_FileDialogToParent:
        return QQFactory::icon(QStringLiteral("up"));
    case SP_FileIcon:
        return QQFactory::icon(QStringLiteral("file"));
    default:
        return QProxyStyle::standardIcon(standardIcon, option, widget);
    }
}

/**
 * Returns the sub-area for the given element as described in the provided
 * style option. The returned rectangle is defined in screen coordinates.
 * Overridden to give some left padding to QAbstractItemView and QHeaderView
 * cells.
 *
 * @param element The type of element being drawn
 * @param option The configuration of the element being drawn
 * @param widget The widget being drawn
 * @return The specified sub-element's screen area
 */
QRect QQAndroidStyle::subElementRect(
        SubElement element, const QStyleOption *option,
        const QWidget *widget) const
{
    QRect rect = QProxyStyle::subElementRect(element, option, widget);
    if (element == SE_ItemViewItemText) {
        rect.setLeft(rect.left() + QQFactory::dpToPixels(4));
    }
    return rect;
}

#endif // Q_OS_ANDROID
