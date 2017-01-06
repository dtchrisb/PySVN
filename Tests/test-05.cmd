@echo off
@prompt $P$S$G
@rem
@rem   test-05.cmd
@rem       test info2 with locked files
@rem
@echo WorkDir: %BUILDER_TOP_DIR%
@echo PYTHON: %PYTHON%
@echo Username: %USERNAME%

setlocal
set PYTHONPATH=%BUILDER_TOP_DIR%\Source;%BUILDER_TOP_DIR%\Examples\Client
set PYSVN=%PYTHON% %BUILDER_TOP_DIR%\Examples\Client\svn_cmd.py --pysvn-testing 01.03.00 --config-dir b:\configdir
echo Info: PYSVN CMD %PYSVN%
call :cmd_shell mkdir testroot-05
call :cmd_shell subst b: %CD%\testroot-05
call :cmd_shell cd /d b:\

call :cmd_shell svnadmin create b:\repos

echo Info: Setup - mkdir
call :cmd_pysvn mkdir file:///b:/repos/trunk -m "test-05 add trunk"
call :cmd_pysvn mkdir file:///b:/repos/trunk/test -m "test-05 add test"

echo Info: Setup - checkout wc1
call :cmd_pysvn checkout file:///b:/repos/trunk b:\wc1
call :cmd_shell cd b:\wc1\test

echo Info: Setup - add files
call :cmd_createfile file1.txt test add file 1
call :cmd_createfile file2.txt test add file 2
call :cmd_pysvn add file1.txt
call :cmd_pysvn add file2.txt
call :cmd_pysvn checkin -m "commit added files"

echo Info: Setup - checkout wc2
call :cmd_pysvn checkout file:///b:/repos/trunk b:\wc2

echo Info: Test - status of unlocked files
call :cmd_pysvn status --verbose b:\wc1

echo Info: Test - info2 of unlocked files
call :cmd_pysvn info2 b:\wc1\test\file1.txt

echo Info: Test - list of unlocked files
call :cmd_pysvn list --verbose --fetch-locks b:\wc1\test\file1.txt
call :cmd_pysvn list --verbose --fetch-locks b:\wc1\test

echo Info: Test - lock unlocked file
call :cmd_pysvn lock b:\wc1\test\file1.txt

echo Info: Test - status of locked files
call :cmd_pysvn status --verbose b:\wc1

echo Info: Test - info2 of locked files
call :cmd_pysvn info2 b:\wc1\test\file1.txt

echo Info: Test - list of locked files
call :cmd_pysvn list --verbose --fetch-locks b:\wc1\test\file1.txt
call :cmd_pysvn list --verbose --fetch-locks b:\wc1\test

echo Info: Test - attempt to checkin over a locked file
call :cmd_shell cd b:\wc2\test
call :cmd_appendfile file1.txt Change to file 1
call :cmd_appendfile file2.txt Change to file 2
call :cmd_pysvn commit -m "change when file locked in other wc" .

echo Info: Test - lock locked file
call :cmd_pysvn lock b:\wc2\test\file1.txt

echo Info: Test - lock --force locked file
call :cmd_pysvn lock --force b:\wc2\test\file1.txt -m "Stealing lock"

echo Info: Test - info2 of locked files
call :cmd_pysvn info2 b:\wc2\test\file1.txt

echo Info: Test - status of locked files
call :cmd_pysvn status --verbose b:\wc2

echo Info: Test - list of locked files
call :cmd_pysvn list --verbose --fetch-locks --recursive b:\wc2

echo Info: Test - commit with lock
call :cmd_pysvn commit -m "change when file locked in this wc" .

echo Info: Test - status of locked files
call :cmd_pysvn status --verbose b:\wc2

echo Info: Test - list of locked files
call :cmd_pysvn list --verbose --fetch-locks --recursive b:\wc2

echo Info: Test - unlock locked file
call :cmd_pysvn unlock b:\wc2\test\file1.txt

echo Info: Test - status of unlocked files
call :cmd_pysvn status --verbose b:\wc2

echo Info: Test - list of unlocked files
call :cmd_pysvn list --verbose --fetch-locks --recursive b:\wc2

echo Info: Test - status of locked files
call :cmd_pysvn status --verbose b:\wc1

echo Info: Test - list of unlocked files
call :cmd_pysvn list --verbose --fetch-locks --recursive b:\wc1

echo Info: Test - update with stolen lock
call :cmd_pysvn update b:\wc1\test

echo Info: Test - status of locked files
call :cmd_pysvn status --verbose b:\wc1

echo Info: Test - info2 of URL
call :cmd_pysvn info2 --revision HEAD file:///b:/repos/trunk/test/file1.txt

echo Info: Test - list of locked files
call :cmd_pysvn list --verbose --fetch-locks --recursive b:\wc1

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
