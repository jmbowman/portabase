TEMPLATE        = app
CONFIG         += qt warn_on release
TRANSLATIONS    = portabase_ja.ts \
                  portabase_tw.ts \
                  portabase_fr.ts \
                  portabase_cs.ts
LIBS           += mk4vc60s.lib beecrypt.lib
TARGET          = portabase
DEFINES        += DESKTOP QT_DLL
TMAKE_CXXFLAGS += /MD
RC_FILE         = portabase.rc
HEADERS         = portabase.h \
                  view.h \
                  viewdisplay.h \
                  database.h \
                  datatypes.h \
                  dbeditor.h \
                  columneditor.h \
                  roweditor.h \
                  csvutils.h \
                  vieweditor.h \
                  noteeditor.h \
                  notebutton.h \
                  sorteditor.h \
                  condition.h \
                  filter.h \
                  conditioneditor.h \
                  filtereditor.h \
                  datewidget.h \
                  preferences.h \
                  enumeditor.h \
                  enummanager.h \
                  timewidget.h \
                  pdbfile.h \
                  mobiledb.h \
                  importutils.h \
                  rowviewer.h \
                  QtaDatePicker.h \
                  desktop/applnk.h \
                  desktop/config.h \
                  desktop/filemanager.h \
                  desktop/fileselector.h \
                  desktop/importdialog.h \
                  desktop/qpeapplication.h \
                  desktop/qpemenubar.h \
                  desktop/resource.h \
                  desktop/timestring.h \
                  desktop/helpbrowser.h \
                  xmlexport.h \
                  xmlimport.h \
                  commandline.h \
                  bytestream.h \
                  crypto.h \
                  passdialog.h \
                  csverror.h \
                  shadedlistitem.h \
                  colorbutton.h \
                  calculator.h \
                  numberwidget.h \
                  dynamicedit.h \
                  calc/calcnode.h \
                  calc/calcnodeeditor.h \
                  calc/calcdateeditor.h \
                  calc/calceditor.h \
                  calc/calcwidget.h \
                  desktop/newfiledialog.h \
                  pbdialog.h \
                  calc/calctimeeditor.h \
                  menuactions.h
SOURCES         = main.cpp \
                  portabase.cpp \
                  view.cpp \
                  viewdisplay.cpp \
                  database.cpp \
                  dbeditor.cpp \
                  columneditor.cpp \
                  roweditor.cpp \
                  csvutils.cpp \
                  vieweditor.cpp \
                  noteeditor.cpp \
                  notebutton.cpp \
                  sorteditor.cpp \
                  condition.cpp \
                  filter.cpp \
                  conditioneditor.cpp \
                  filtereditor.cpp \
                  datewidget.cpp \
                  preferences.cpp \
                  enumeditor.cpp \
                  enummanager.cpp \
                  timewidget.cpp \
                  pdbfile.cpp \
                  mobiledb.cpp \
                  importutils.cpp \
                  rowviewer.cpp \
                  QtaDatePicker.cpp \
                  desktop/applnk.cpp \
                  desktop/config.cpp \
                  desktop/filemanager.cpp \
                  desktop/fileselector.cpp \
                  desktop/importdialog.cpp \
                  desktop/qpeapplication.cpp \
                  desktop/qpemenubar.cpp \
                  desktop/resource.cpp \
                  desktop/timestring.cpp \
                  desktop/helpbrowser.cpp \
                  xmlexport.cpp \
                  xmlimport.cpp \
                  commandline.cpp \
                  bytestream.cpp \
                  crypto.cpp \
                  passdialog.cpp \
                  csverror.cpp \
                  shadedlistitem.cpp \
                  colorbutton.cpp \
                  calculator.cpp \
                  numberwidget.cpp \
                  dynamicedit.cpp \
                  calc/calcnode.cpp \
                  calc/calcnodeeditor.cpp \
                  calc/calcdateeditor.cpp \
                  calc/calceditor.cpp \
                  calc/calcwidget.cpp \
                  desktop/newfiledialog.cpp \
                  pbdialog.cpp \
                  calc/calctimeeditor.cpp \
                  menuactions.cpp
