/*
 * main.cpp
 *
 * (c) 2002-2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#if defined(DESKTOP)
#include "desktop/qpeapplication.h"
#else
#include <qpe/qpeapplication.h>
#endif

#include "portabase.h"

int main(int argc, char **argv) {
    QPEApplication app(argc, argv);
    PortaBase pb;
    pb.setCaption("PortaBase");
    app.showMainDocumentWidget(&pb);
    if (argc == 3 && argv[1] == QCString("-f")) {
        pb.openFile(argv[2]);
    }
    return app.exec();
}
