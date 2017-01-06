@echo off
@prompt $P$S$G
@rem
@rem   test-07.cmd
@rem       test copy2, move2 and changelist
@rem
@echo WorkDir: %BUILDER_TOP_DIR%
@echo PYTHON: %PYTHON%
@echo Username: %USERNAME%

setlocal
set PYTHONPATH=%BUILDER_TOP_DIR%\Source;%BUILDER_TOP_DIR%\Examples\Client
set PYSVN=%PYTHON% %BUILDER_TOP_DIR%\Examples\Client\svn_cmd.py --pysvn-testing 01.03.00 --config-dir b:\configdir
echo Info: PYSVN CMD %PYSVN%

call :cmd_shell mkdir testroot-07
call :cmd_shell subst b: %CD%\testroot-07
call :cmd_shell cd /d b:\

call :cmd_shell svnadmin create b:/repos

echo Info: Setup - mkdir
call :cmd_pysvn mkdir file:///b:/repos/trunk -m "test-07 add trunk"
call :cmd_pysvn mkdir file:///b:/repos/trunk/test -m "test-07 add test"

echo Info: Setup - checkout wc1
call :cmd_pysvn checkout file:///b:/repos/trunk b:/wc1
call :cmd_shell cd /d b:/wc1/test

echo Info: Setup - add files

call :cmd_createfile file_a1.txt test add file 1
call :cmd_pysvn add file_a1.txt
call :cmd_pysvn checkin -m "commit added files"
call :cmd_createfile file_a2.txt test add file 2
call :cmd_pysvn add file_a2.txt
call :cmd_pysvn checkin -m "commit added files"

call :cmd_createfile file_b1.txt test add file 1
call :cmd_pysvn add file_b1.txt
call :cmd_pysvn checkin -m "commit added files"
call :cmd_createfile file_b2.txt test add file 2
call :cmd_pysvn add file_b2.txt
call :cmd_pysvn checkin -m "commit added files"

call :cmd_pysvn status --verbose b:/wc1

echo Info: running test_07_copy2
%PYTHON% %BUILDER_TOP_DIR%/Tests/test_07_copy2.py b:/configdir

echo Info: running test_07_move2
%PYTHON% %BUILDER_TOP_DIR%/Tests/test_07_move2.py b:/configdir

echo Info: running test_07_changelist
%PYTHON% %BUILDER_TOP_DIR%/Tests/test_07_changelist.py b:/configdir

rem echo Info: running test_07_revprops
rem %PYTHON% %BUILDER_TOP_DIR%/Tests/test_07_revprops.py b:/configdir

goto :eof
endlocal

:cmd_shell
    echo.
    echo Info: CMD %*
    %*
    goto :eof

:cmd_pysvn
    echo.
    echo Info: PYSVN CMD %*
    %PYSVN% %*
    goto :eof

:cmd_createfile
    set FILENAME=%1
    shift
    echo Info: Create File %FILENAME% - %1 %2 %3 %4 %5 %6 %7 %8 %9
    call :cmd__echo %1 %2 %3 %4 %5 %6 %7 %8 %9 >%FILENAME%
    goto :eof

:cmd_appendfile
    set FILENAME=%1
    shift
    echo Info: Append File %FILENAME% - %1 %2 %3 %4 %5 %6 %7 %8 %9
    call :cmd__echo %1 %2 %3 %4 %5 %6 %7 %8 %9 >>%FILENAME%
    goto :eof

:cmd__echo
    echo %*
    goto :eof
