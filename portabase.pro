TEMPLATE        = app
#CONFIG         = qt warn_on debug
CONFIG          = qt warn_on release
HEADERS         = portabase.h inputdialog.h view.h viewdisplay.h database.h datatypes.h dbeditor.h columneditor.h roweditor.h csvutils.h importdialog.h vieweditor.h noteeditor.h notebutton.h sorteditor.h condition.h filter.h conditioneditor.h filtereditor.h datewidget.h QtaDatePicker.h preferences.h enumeditor.h enummanager.h timewidget.h pdbfile.h mobiledb.h importutils.h rowviewer.h xmlexport.h
SOURCES         = main.cpp portabase.cpp inputdialog.cpp view.cpp viewdisplay.cpp database.cpp dbeditor.cpp columneditor.cpp roweditor.cpp csvutils.cpp importdialog.cpp vieweditor.cpp noteeditor.cpp notebutton.cpp sorteditor.cpp condition.cpp filter.cpp conditioneditor.cpp filtereditor.cpp datewidget.cpp QtaDatePicker.cpp preferences.cpp enumeditor.cpp enummanager.cpp timewidget.cpp pdbfile.cpp mobiledb.cpp importutils.cpp rowviewer.cpp xmlexport.cpp
TRANSLATIONS    = portabase_jp.ts
INCLUDEPATH     += $(QPEDIR)/include
DEPENDPATH      += $(QPEDIR)/include
LIBS            += -lqpe -lm -lmk4
TARGET          = portabase
