TEMPLATE        = app
CONFIG         += qt warn_on release
TARGET          = portabase
TRANSLATIONS    = portabase_cs.ts \
                  portabase_fr.ts \
                  portabase_ja.ts \
                  portabase_zh_TW.ts
# determine which version to compile for
# options are desktop, qtopia, or sharp (add "test" for the test environment)
CONFIG         += desktop

# Files common between all versions
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
                  calc/calcnode.h \
                  calc/calcnodeeditor.h \
                  calc/calcdateeditor.h \
                  calc/calceditor.h \
                  calc/calcwidget.h \
                  pbdialog.h \
                  calc/calctimeeditor.h \
                  menuactions.h \
                  image/imageeditor.h \
                  image/imagereader.h \
                  image/imageselector.h \
                  image/imageutils.h \
                  image/imageviewer.h \
                  image/imagewidget.h \
                  image/slideshowdialog.h \
                  qqdialog.h \
                  metakitfuncs.h
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
                  calc/calcnode.cpp \
                  calc/calcnodeeditor.cpp \
                  calc/calcdateeditor.cpp \
                  calc/calceditor.cpp \
                  calc/calcwidget.cpp \
                  pbdialog.cpp \
                  calc/calctimeeditor.cpp \
                  menuactions.cpp \
                  image/imageeditor.cpp \
                  image/imagereader.cpp \
                  image/imageselector.cpp \
                  image/imageutils.cpp \
                  image/imageviewer.cpp \
                  image/imagewidget.cpp \
                  image/slideshowdialog.cpp \
                  qqdialog.cpp \
                  metakitfuncs.cpp

# Stuff for all Linux/UNIX versions
unix:LIBS       += -lm -lmk4 -lbeecrypt -ljpeg

# Stuff for older Zaurus ROMs
qtopia:CONFIG  += zaurus
qtopia:HEADERS += importdialog.h \
                  newfiledialog.h \
                  fileselector.h
qtopia:SOURCES += importdialog.cpp \
                  newfiledialog.cpp \
                  fileselector.cpp

# Stuff for recent Zaurus ROMs
sharp:DEFINES  += SHARP
sharp:LIBS     += -lsl -lzdtm
sharp:CONFIG   += zaurus
sharp:HEADERS  += sharp/importdialog.h \
                  sharp/newfiledialog.h \
                  sharp/fileselector.h
sharp:SOURCES  += sharp/importdialog.cpp \
                  sharp/newfiledialog.cpp \
                  sharp/fileselector.cpp

# Stuff for all Zaurus versions
zaurus:INCLUDEPATH += $(QPEDIR)/include
zaurus:DEPENDPATH  += $(QPEDIR)/include
zaurus:LIBS        += -lqpe
zaurus:HEADERS     += QtaDatePicker.h \
                      colordialog.h \
                      dynamicedit.h \
                      inputdialog.h
zaurus:SOURCES     += QtaDatePicker.cpp \
                      colordialog.cpp \
                      dynamicedit.cpp \
                      inputdialog.cpp

# Stuff for the Qtopia test environment
test:TMAKE_CC   = gcc-2.95
test:TMAKE_CXX  = g++-2.95

# Stuff for all desktop versions
desktop:CONFIG  += thread
desktop:HEADERS += desktop/QtaDatePicker.h \
                   desktop/applnk.h \
                   desktop/config.h \
                   desktop/dynamicedit.h \
                   desktop/filemanager.h \
                   desktop/fileselector.h \
                   desktop/helpbrowser.h \
                   desktop/importdialog.h \
                   desktop/newfiledialog.h \
                   desktop/oldconfig.h \
                   desktop/qpeapplication.h \
                   desktop/resource.h \
                   desktop/timestring.h
desktop:SOURCES += desktop/QtaDatePicker.cpp \
                   desktop/applnk.cpp \
                   desktop/config.cpp \
                   desktop/dynamicedit.cpp \
                   desktop/filemanager.cpp \
                   desktop/fileselector.cpp \
                   desktop/helpbrowser.cpp \
                   desktop/importdialog.cpp \
                   desktop/newfiledialog.cpp \
                   desktop/oldconfig.cpp \
                   desktop/qpeapplication.cpp \
                   desktop/resource.cpp \
                   desktop/timestring.cpp

# Stuff for Windows
win32:LIBS                   += mk4vc60s.lib beecrypt.lib libjpeg.lib
win32:DEFINES                += QT_DLL
win32:RC_FILE                 = portabase.rc
win32:QMAKE_CXXFLAGS_RELEASE += /MD
win32:INCLUDEPATH            += D:\Devel\metakit-2.4.9.3\include \
                                D:\Devel\jpeg-6b \
                                D:\Devel
