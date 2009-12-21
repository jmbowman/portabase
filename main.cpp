/*
 * main.cpp
 *
 * (c) 2002-2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "commandline.h"
#include "portabase.h"

#if defined(Q_WS_QWS)
#include <qpe/qpeapplication.h>
#else
#include "desktop/qpeapplication.h"
#endif

#if defined(Q_OS_MACX)
#include "mac/eventhandlers.h"
#endif

int main(int argc, char **argv) {
    QPEApplication app(argc, argv);
    if (argc > 1 && argv[1] != QCString("-f")
            && argv[1] != QCString("-qcop")) {
        CommandLine commandLine;
        return commandLine.process(argc, argv);
    }
    else {
        PortaBase pb;
        pb.setCaption("PortaBase");
        app.showMainDocumentWidget(&pb);
        if (argc == 3 && argv[1] == QCString("-f")) {
            pb.openFile(argv[2]);
        }
#if defined(Q_OS_MACX)
        pbInstance = &pb;
        // need this in order to open files launched from Finder
        AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments,
                              NewAEEventHandlerUPP(odocHandler), 0, false);
#endif
        return app.exec();
    }
}
