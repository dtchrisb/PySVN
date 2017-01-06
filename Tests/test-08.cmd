@echo off
@prompt $P$S$G
@rem
@rem   test-08.cmd
@rem       test patch
@rem

@echo WorkDir: %BUILDER_TOP_DIR%
@echo PYTHON: %PYTHON%
@echo Username: %USERNAME%

setlocal
set PYTHONPATH=%BUILDER_TOP_DIR%\Source;%BUILDER_TOP_DIR%\Examples\Client
set PYSVN=%PYTHON% %BUILDER_TOP_DIR%\Examples\Client\svn_cmd.py --pysvn-testing 01.01.00 --config-dir b:\configdir
echo Info: PYSVN CMD %PYSVN%
call :cmd_shell mkdir testroot-08
call :cmd_shell subst b: %CD%\testroot-08
call :cmd_shell cd /d b:\

call :cmd_shell mkdir tmp
set TMPDIR=b:\tmp

call :cmd_shell svnadmin create b:\repos

echo Info: Setup - mkdir
call :cmd_pysvn mkdir file:///b:/repos/trunk -m "test-08 add trunk"

echo Info: Setup - checkout wc1
call :cmd_pysvn checkout file:///b:/repos/trunk b:\wc1
call :cmd_shell cd /d b:\wc1

echo Info: Setup - add files and folders

call :cmd_pysvn mkdir folder1
call :cmd_pysvn mkdir folder2
call :cmd_pysvn mkdir folder2\sub2

call :cmd_createfile folder1\file-a.txt test add file 1
call :cmd_pysvn add folder1\file-a.txt

call :cmd_createfile folder2\file-b.txt test add file 2
call :cmd_pysvn add folder2\file-b.txt

call :cmd_createfile folder2\sub2\file-b.txt test add file 2
call :cmd_pysvn add folder2\sub2\file-b.txt

call :cmd_pysvn checkin -m "commit added files"

call :cmd_appendfile folder1\file-a.txt test add line 2
call :cmd_pysvn diff folder1\file-a.txt >b:\diff-1.patch

call :cmd_shell type b:\diff-1.patch

call :cmd_shell type folder1\file-a.txt

call :cmd_pysvn checkout file:///b:/repos/trunk b:\wc2

call :cmd_shell cd /d b:\wc2

call :cmd_shell type folder1\file-a.txt

echo on
call :cmd_pysvn patch b:\diff-1.patch b:\wc2 --no-remove-tempfiles

call :cmd_shell type folder1\file-a.txt

call :cmd_shell fc b:\wc1\folder1\file-a.txt b:\wc2\folder1\file-a.txt

goto :eof
endlocal

:cmd_shell
    echo.
    echo Info: CMD %*
    %*
    goto :eof

:cmd_pysvn
    echo. 1>&2
    echo Info: PYSVN CMD %* 1>&2
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
