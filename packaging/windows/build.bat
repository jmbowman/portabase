qmake portabase.pro
lrelease portabase.pro
lrelease resources\translations\qt*.ts
mingw32-make
IF NOT "%1" EQU "" (
    "\Program Files\kSign\kSignCMD.exe" /d "PortaBase" /du "http://www.portabase.org" /f "\Users\Jeremy Bowman\Documents\jb_certificate.pfx" /p "%1" build\PortaBase.exe
)
