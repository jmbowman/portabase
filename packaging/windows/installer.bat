SET mingw_dir=C:\Qt\2010.05\mingw
SET qt_dir=C:\Qt\4.7.1
IF NOT EXIST build\windows MKDIR build\windows
COPY CHANGES build\windows\CHANGES.txt
COPY COPYING build\windows\COPYING.txt
COPY README.txt build\windows
packaging\windows\UNIX2DOS build\windows\CHANGES.txt
packaging\windows\UNIX2DOS build\windows\COPYING.txt
packaging\windows\UNIX2DOS build\windows\README.txt
COPY %mingw_dir%\bin\mingwm10.dll build\windows
COPY %mingw_dir%\bin\libgcc_s_dw2-1.dll build\windows
COPY %qt_dir%\bin\QtCore4.dll build\windows
COPY %qt_dir%\bin\QtGui4.dll build\windows
COPY %qt_dir%\bin\QtXml4.dll build\windows
IF NOT EXIST build\windows\accessible MKDIR build\windows\accessible
copy %qt_dir%\plugins\accessible\qtaccessiblewidgets4.dll build\windows\accessible
IF NOT EXIST build\windows\imageformats MKDIR build\windows\imageformats
copy %qt_dir%\plugins\imageformats\qjpeg4.dll build\windows\imageformats
"C:\Program Files\Inno Setup 5\ISCC" portabase.iss
