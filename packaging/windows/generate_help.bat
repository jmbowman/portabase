REM Generate help files; separate script because it mysteriously exits at end of for loop

CD resources\help
IF EXIST _static RD /S /Q _static
FOR /F %%G IN ('dir /a:d /b') DO (
    CD %%G
    .\make clean
    .\make html
    IF /I "%%G" EQU "en" (MOVE _build\html\_static ..) ELSE (RD /S /Q _build\html\_static)
    python ..\..\..\packaging\fix_help_paths.py _build\html
    CD ..)
