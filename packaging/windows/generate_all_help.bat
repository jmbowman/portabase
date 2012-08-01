REM Generate help files for all languages

CALL packaging\windows\generate_help en
C:\Python27\python packaging\fix_help_paths.py resources\help\_build\html
CD resources\help\_build
MOVE html\_static .
MOVE html en
CD ..\..\..
FOR /F %%G IN ('DIR /A:D /B resources\help\translations') DO (
    IF /I "%%G" NEQ "templates" (
        CALL packaging\windows\generate_help --no-clean %%G
        C:\Python27\python packaging\fix_help_paths.py resources\help\_build\html
        CD resources\help\_build
        MOVE html\_static\translations.js html
        RD /S /Q html\_static
        MOVE html %%G
        CD ..\..\..
    )
)
