@echo off
@prompt $P$S$G
@rem
@rem   test-06.cmd
@rem       test info and info2
@rem
@echo WorkDir: %BUILDER_TOP_DIR%
@echo PYTHON: %PYTHON%
@echo Username: %USERNAME%

setlocal
set PYTHONPATH=%BUILDER_TOP_DIR%\Source;%BUILDER_TOP_DIR%\Examples\Client
set PYSVN=%PYTHON% %BUILDER_TOP_DIR%\Examples\Client\svn_cmd.py --pysvn-testing 01.03.00 --config-dir b:\configdir
echo Info: PYSVN CMD %PYSVN%
call :cmd_shell mkdir testroot-06
call :cmd_shell subst b: %CD%\testroot-06
call :cmd_shell cd /d b:\

call :cmd_shell svnadmin create b:/repos

echo Info: Setup - mkdir
call :cmd_pysvn mkdir file:///b:/repos/trunk -m "test-06 add trunk"
call :cmd_pysvn mkdir file:///b:/repos/trunk/test -m "test-06 add test"

echo Info: Setup - checkout wc1
call :cmd_pysvn checkout file:///b:/repos/trunk b:\wc1
call :cmd_shell cd b:\wc1\test

echo Info: Setup - add files
call :cmd_createfile file1.txt test add file 1
call :cmd_createfile file2.txt test add file 2
call :cmd_pysvn add file1.txt
call :cmd_pysvn add file2.txt
call :cmd_pysvn checkin -m "commit added files"

echo Info: Test - info of path
call :cmd_pysvn info file1.txt

echo Info: Test - info2 of path
call :cmd_pysvn info2 file1.txt

echo Info: Test - info2 of URL
call :cmd_pysvn info2 --revision HEAD file:///b:/repos/trunk/test/file1.txt

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
