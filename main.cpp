/*
 * main.cpp
 *
 * (c) 2002 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qpe/qpeapplication.h>
#include "portabase.h"

int main(int argc, char **argv) {
    QPEApplication app(argc, argv);
    PortaBase pb;
    pb.setCaption("PortaBase");
    app.showMainDocumentWidget(&pb);
    if (argc == 3 && argv[1] == QCString("-f")) {
        pb.openFile(argv[2]);
    }
    //app.connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
    return app.exec();
}
