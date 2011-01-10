/*
 * pbmaemo5style.cpp
 *
 * (c) 2011 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file
 * Source file for PBMaemo5Style
 */

#include "pbmaemo5style.h"

#if defined(Q_WS_MAEMO_5)

/**
 * Constructor.
 */
PBMaemo5Style::PBMaemo5Style() : QMaemo5Style(), variableHeightRows(false)
{

}

/**
 * Set whether or not rows in item views are allowed to vary from
 * finger-friendly height.
 *
 * @param bool y True if rows are allowed to vary in height, false otherwise
 */
void PBMaemo5Style::setVariableHeightRows(bool y)
{
    variableHeightRows = y;
}

/**
 * Returns the size of the element described by the specified option and
 * type, based on the provided contentsSize.
 *
 * @param type The type of element being sized
 * @param option A pointer to a QStyleOption or one of its subclasses
 * @param size Optionally pre-calculated content size
 * @param widget The widget in question, which can contain extra information
 *               used for calculating the size
 * @return The suggested size for the element
 */
QSize PBMaemo5Style::sizeFromContents(ContentsType type, const QStyleOption *option,
                                  const QSize &size, const QWidget *widget) const
{
    if (variableHeightRows && type == CT_ItemViewItem) {
        return QGtkStyle::sizeFromContents(type, option, size, widget);
    }
    return QMaemo5Style::sizeFromContents(type, option, size, widget);
}
#endif
