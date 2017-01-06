echo on
setlocal
set PYVER=%1

set PYTHONPATH=%BUILDER_TOP_DIR%/Source:%BUILDER_TOP_DIR%/Examples/Client
py -%PY_VER% %BUILDER_TOP_DIR%/Examples/Client/svn_cmd.py %2 %3 %4 %5 %6 %7 %8 %9
endlocal
