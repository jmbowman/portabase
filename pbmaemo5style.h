/*
 * pbmaemo5style.h
 *
 * (c) 2011 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file
 * Header file for PBMaemo5Style
 */

#include <QString>
#if defined(Q_WS_MAEMO_5)
#ifndef PB_MAEMO_5_STYLE_H
#define PB_MAEMO_5_STYLE_H

#include <QMaemo5Style>

/**
 * Style overrides for PortaBase on Maemo 5 (Fremantle).  So far, it just
 * provides the option of overriding the finger-friendly height of item view
 * rows in order to fit more rows on the screen.
 */
class PBMaemo5Style: public QMaemo5Style
{
public:
    PBMaemo5Style();

    void setVariableHeightRows(bool y);
    QSize sizeFromContents(ContentsType type, const QStyleOption *option,
                           const QSize &size, const QWidget *widget) const;

private:
    bool variableHeightRows; /**< True if rows are allowed to deviate from finger-friendly height */
};
#endif // PB_MAEMO_5_STYLE_H
#endif // Q_WS_MAEMO_5
