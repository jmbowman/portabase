TEMPLATE        = app
CONFIG         += qt warn_on release thread
HEADERS         = portabase.h view.h viewdisplay.h database.h datatypes.h dbeditor.h columneditor.h roweditor.h csvutils.h vieweditor.h noteeditor.h notebutton.h sorteditor.h condition.h filter.h conditioneditor.h filtereditor.h datewidget.h preferences.h enumeditor.h enummanager.h timewidget.h pdbfile.h mobiledb.h importutils.h rowviewer.h desktop/QtaDatePicker.h desktop/applnk.h desktop/config.h desktop/filemanager.h desktop/fileselector.h desktop/importdialog.h desktop/qpeapplication.h desktop/qpemenubar.h desktop/resource.h desktop/timestring.h desktop/helpbrowser.h xmlexport.h
SOURCES         = main.cpp portabase.cpp view.cpp viewdisplay.cpp database.cpp dbeditor.cpp columneditor.cpp roweditor.cpp csvutils.cpp vieweditor.cpp noteeditor.cpp notebutton.cpp sorteditor.cpp condition.cpp filter.cpp conditioneditor.cpp filtereditor.cpp datewidget.cpp preferences.cpp enumeditor.cpp enummanager.cpp timewidget.cpp pdbfile.cpp mobiledb.cpp importutils.cpp rowviewer.cpp desktop/QtaDatePicker.cpp desktop/applnk.cpp desktop/config.cpp desktop/filemanager.cpp desktop/fileselector.cpp desktop/importdialog.cpp desktop/qpeapplication.cpp desktop/qpemenubar.cpp desktop/resource.cpp desktop/timestring.cpp desktop/helpbrowser.cpp xmlexport.cpp
TRANSLATIONS    = portabase_jp.ts
LIBS            += -lm -lmk4
TARGET          = portabase
DEFINES        += DESKTOP
