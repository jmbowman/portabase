TEMPLATE        = app
#CONFIG         = qt warn_on debug
CONFIG          = qt warn_on release
TRANSLATIONS    = portabase_ja.ts \
                  portabase_tw.ts \
                  portabase_fr.ts \
                  portabase_cs.ts
TMAKE_CC        = gcc-2.95
TMAKE_CXX       = g++-2.95
INCLUDEPATH     += $(QPEDIR)/include
DEPENDPATH      += $(QPEDIR)/include
LIBS            += -lqpe -lm -lmk4 -lbeecrypt
TARGET          = portabase
HEADERS         = portabase.h \
                  inputdialog.h \
                  view.h \
                  viewdisplay.h \
                  database.h \
                  datatypes.h \
                  dbeditor.h \
                  columneditor.h \
                  roweditor.h \
                  csvutils.h \
                  importdialog.h \
                  vieweditor.h \
                  noteeditor.h \
                  notebutton.h \
                  sorteditor.h \
                  condition.h \
                  filter.h \
                  conditioneditor.h \
                  filtereditor.h \
                  datewidget.h \
                  QtaDatePicker.h \
                  preferences.h \
                  enumeditor.h \
                  enummanager.h \
                  timewidget.h \
                  pdbfile.h \
                  mobiledb.h \
                  importutils.h \
                  rowviewer.h \
                  xmlexport.h \
                  xmlimport.h \
                  commandline.h \
                  bytestream.h \
                  crypto.h \
                  passdialog.h \
                  newfiledialog.h \
                  csverror.h \
                  shadedlistitem.h \
                  colordialog.h \
                  colorbutton.h \
                  calculator.h \
                  numberwidget.h \
                  dynamicedit.h \
                  calc/calcnode.h \
                  calc/calcnodeeditor.h \
                  calc/calcdateeditor.h \
                  calc/calceditor.h \
                  calc/calcwidget.h \
                  fileselector.h \
                  pbdialog.h \
                  calc/calctimeeditor.h \
                  menuactions.h
SOURCES         = main.cpp \
                  portabase.cpp \
                  inputdialog.cpp \
                  view.cpp \
                  viewdisplay.cpp \
                  database.cpp \
                  dbeditor.cpp \
                  columneditor.cpp \
                  roweditor.cpp \
                  csvutils.cpp \
                  importdialog.cpp \
                  vieweditor.cpp \
                  noteeditor.cpp \
                  notebutton.cpp \
                  sorteditor.cpp \
                  condition.cpp \
                  filter.cpp \
                  conditioneditor.cpp \
                  filtereditor.cpp \
                  datewidget.cpp \
                  QtaDatePicker.cpp \
                  preferences.cpp \
                  enumeditor.cpp \
                  enummanager.cpp \
                  timewidget.cpp \
                  pdbfile.cpp \
                  mobiledb.cpp \
                  importutils.cpp \
                  rowviewer.cpp \
                  xmlexport.cpp \
                  xmlimport.cpp \
                  commandline.cpp \
                  bytestream.cpp \
                  crypto.cpp \
                  passdialog.cpp \
                  newfiledialog.cpp \
                  csverror.cpp \
                  shadedlistitem.cpp \
                  colordialog.cpp \
                  colorbutton.cpp \
                  calculator.cpp \
                  numberwidget.cpp \
                  dynamicedit.cpp \
                  calc/calcnode.cpp \
                  calc/calcnodeeditor.cpp \
                  calc/calcdateeditor.cpp \
                  calc/calceditor.cpp \
                  calc/calcwidget.cpp \
                  fileselector.cpp \
                  pbdialog.cpp \
                  calc/calctimeeditor.cpp \
                  menuactions.cpp
