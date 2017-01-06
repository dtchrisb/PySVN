@echo off
@prompt $P$S$G
@rem
@rem   test-01.cmd
@rem       test the main commands that are in all versions
@rem
@echo WorkDir: %BUILDER_TOP_DIR%
@echo PYTHON: %PYTHON%
@echo Username: %USERNAME%

setlocal
set PYTHONPATH=%BUILDER_TOP_DIR%\Source;%BUILDER_TOP_DIR%\Examples\Client
set PYSVN=%PYTHON% %BUILDER_TOP_DIR%\Examples\Client\svn_cmd.py --pysvn-testing 01.01.00 --config-dir b:\configdir
echo Info: PYSVN CMD %PYSVN%
call :cmd_shell mkdir testroot-01
call :cmd_shell subst b: %CD%\testroot-01
call :cmd_shell cd /d b:\

call :cmd_shell svnadmin create b:\repos

echo Info: Test - mkdir
call :cmd_pysvn mkdir file:///b:/repos/trunk -m "test-01 add trunk"
call :cmd_pysvn mkdir file:///b:/repos/trunk/test -m "test-01 add test"

echo Info: Test - ls
call :cmd_pysvn ls file:///b:/repos -v -R

echo Info: Test - checkout
call :cmd_pysvn checkout file:///b:/repos/trunk b:\wc1
call :cmd_shell dir b:\wc1 /s /b /a-h
call :cmd_shell cd /d b:\wc1\test

echo Info: Test - add
call :cmd_createfile file1.txt test add file 1
call :cmd_createfile file2.txt test add file 2
call :cmd_createfile file3.txt test add file 3
call :cmd_createfile file4.txt test add file 4
call :cmd_createfile file5.txt test add file 5
call :cmd_shell mkdir folder1
call :cmd_createfile folder1\file7.txt test add file 7
call :cmd_shell mkdir folder1\folder2
call :cmd_createfile folder1\folder2\file8.txt test add file 8
call :cmd_shell mkdir folder3
call :cmd_createfile folder3\file9.txt test add file 9
call :cmd_shell mkdir folder3\folder4
call :cmd_createfile folder3\folder4\file10.txt test add file 10

call :cmd_pysvn add file1.txt
call :cmd_pysvn add file2.txt
call :cmd_pysvn add file3.txt
call :cmd_pysvn add file4.txt
call :cmd_pysvn add --force file5.txt
call :cmd_pysvn add folder1
call :cmd_pysvn add --non-recursive folder3

call :cmd_pysvn checkin -m "commit added files"

echo Info: Test - update - get a new wc that will update
call :cmd_pysvn checkout file:///b:/repos/trunk b:\wc2

echo Info: Test - - checkin a mod from wc1
call :cmd_appendfile b:\wc1\test\file1.txt line 2
call :cmd_pysvn checkin -m "commit modified file"

call :cmd_pysvn checkin -m "commit modified file"
echo Info: Test - update
call :cmd_pysvn update b:\wc2

echo Info: Test - the rest in lexical order

echo Info: Test - annotate
call :cmd_pysvn annotate b:\wc2\test\file1.txt

echo Info: Test - cat
call :cmd_pysvn cat -r head file:///b:/repos/trunk/test/file1.txt

echo Info: Test - cleanup

echo Info: Test - copy
call :cmd_pysvn mkdir file:///b:/repos/tags -m "test-01 add tags"
call :cmd_createfile msg.tmp tag the trunk
call :cmd_pysvn copy file:///b:/repos/trunk file:///b:/repos/tags/version1 <msg.tmp
call :cmd_pysvn ls -v file:///b:/repos/tags
call :cmd_pysvn copy b:\wc2\test\file1.txt b:\wc2\test\file1b.txt
call :cmd_pysvn propset svn:eol-style native b:\wc2\test\file1b.txt
call :cmd_pysvn checkin b:\wc2 -m "copy test"

echo Info: Test - diff
call :cmd_appendfile b:\wc2\test\file1b.txt new line
call :cmd_pysvn diff b:\wc2

echo Info: Test - export
call :cmd_pysvn export file:///b:/repos/trunk/test b:\export1.native
call :cmd_pysvn export --native-eol CR file:///b:/repos/trunk/test b:\export1.cr
call :cmd_pysvn export --native-eol LF file:///b:/repos/trunk/test b:\export1.lf
call :cmd_pysvn export --native-eol CRLF file:///b:/repos/trunk/test b:\export1.crlf
call :cmd_shell dir /s /b b:\export1.native
call :cmd_shell dir /s /b b:\export1.cr
call :cmd_shell dir /s /b b:\export1.lf
call :cmd_shell dir /s /b b:\export1.crlf

echo Info: Test - info
call :cmd_pysvn info b:\wc2\test
call :cmd_pysvn info b:\wc2\test\file1.txt

echo Info: Test - log
call :cmd_pysvn log b:\wc2

echo Info: Test - ls
call :cmd_pysvn ls file:///b:/repos/trunk/test
call :cmd_pysvn ls -v file:///b:/repos/trunk/test
call :cmd_pysvn ls b:\wc2\test
call :cmd_pysvn ls -v b:\wc2\test

echo Info: Test - merge - see below
echo Info: Test - mkdir - done above

echo Info: Test - move
call :cmd_createfile msg.tmp move url test
call :cmd_pysvn move file:///b:/repos/trunk/test/file2.txt file:///b:/repos/trunk/test/file2b.txt <msg.tmp
call :cmd_pysvn move b:\wc2\test\file3.txt b:\wc2\test\file3b.txt
call :cmd_pysvn checkin b:\wc2 -m "move wc test"


echo Info: Test - status
call :cmd_appendfile b:\wc1\test\file4.txt file 4 is changing
call :cmd_pysvn checkin b:\wc1 -m "change wc1 for status -u to detect"

call :cmd_pysvn status b:\wc2
call :cmd_pysvn status --verbose b:\wc2
call :cmd_pysvn status --show-updates b:\wc2
call :cmd_pysvn status --show-updates --verbose b:\wc2
call :cmd_pysvn update
call :cmd_pysvn status --show-updates b:\wc2
call :cmd_pysvn status --show-updates --verbose b:\wc2
call :cmd_pysvn checkin b:\wc2 -m "prop change"

echo Info: Test - propdel
call :cmd_shell cd /d b:\wc2\test
call :cmd_pysvn propset test:prop1 del_me file4.txt
call :cmd_pysvn proplist -v file4.txt
call :cmd_pysvn propdel test:prop1 file4.txt
call :cmd_pysvn proplist -v file4.txt

echo Info: Test - propget
call :cmd_pysvn propget svn:eol-style file4.txt

echo Info: Test - proplist - see above

echo Info: Test - propset
call :cmd_shell cd /d b:\wc2\test
call :cmd_pysvn proplist -v file4.txt
call :cmd_pysvn propset svn:eol-style native file4.txt
call :cmd_pysvn proplist -v file4.txt

echo Info: Test - remove
call :cmd_shell cd /d b:\wc2\test
call :cmd_pysvn remove file5.txt
call :cmd_pysvn status

echo Info: Test - resolved
call :cmd_appendfile b:\wc1\test\file4.txt conflict in file4 yes
call :cmd_appendfile b:\wc2\test\file4.txt conflict in file4 no
call :cmd_pysvn checkin b:\wc1\test -m "make a conflict part 1"
call :cmd_pysvn update b:\wc2\test
call :cmd_pysvn status
call :cmd_shell copy b:\wc2\test\file4.txt.mine b:\wc2\test\file4.txt
call :cmd_pysvn resolved b:\wc2\test\file4.txt
call :cmd_pysvn checkin b:\wc2\test\file4.txt -m "resolve a confict part 2"

echo Info: Test - revert
call :cmd_pysvn revert file5.txt
call :cmd_pysvn status

echo Info: Test - revproplist
call :cmd_pysvn revproplist file:///b:/repos/trunk

echo Info: Test - revpropget
call :cmd_pysvn revpropget svn:log file:///b:/repos/trunk
call :cmd_pysvn revpropget no_such_prop file:///b:/repos/trunk

echo Info: Test - revpropset
call :cmd_pysvn revpropset svn:log "Hello world" file:///b:/repos/trunk

echo Info: Test - revpropdel
call :cmd_pysvn revpropdel svn:log file:///b:/repos/trunk

echo Info: Test - status - see above

echo Info: Test - relocate
call :cmd_shell mkdir b:\root
call :cmd_shell move b:\repos b:\root
call :cmd_pysvn info b:\wc1
call :cmd_pysvn relocate file:///b:/repos/trunk file:///b:/root/repos/trunk b:\wc1
call :cmd_pysvn info b:\wc1
call :cmd_pysvn info b:\wc2
call :cmd_pysvn relocate file:///b:/repos/trunk file:///b:/root/repos/trunk b:\wc2
call :cmd_pysvn info b:\wc2

echo Info: Test - switch
call :cmd_pysvn info b:\wc2
call :cmd_pysvn switch b:\wc2 file:///b:/root/repos/tags/version1
call :cmd_pysvn info b:\wc2

echo Info: Test - update - see above

echo Info: Test - Info: Testing - merge
call :cmd_pysvn checkout file:///b:/root/repos/trunk b:\wc3
call :cmd_shell cd b:\wc3\test
call :cmd_createfile file-merge-1.txt test add file merge 1
call :cmd_createfile file-merge-2.txt test add file merge 2
call :cmd_pysvn add file-merge-1.txt
call :cmd_pysvn add file-merge-2.txt
call :cmd_pysvn commit -m "add test merge files" .

call :cmd_createfile msg.tmp make a branch
call :cmd_pysvn copy file:///b:/root/repos/trunk/test file:///b:/root/repos/trunk/test-branch <msg.tmp
call :cmd_pysvn update b:\wc3

call :cmd_createfile file-merge-3.txt test add file merge 3
call :cmd_pysvn add file-merge-3.txt
call :cmd_pysvn rm file-merge-1.txt
call :cmd_appendfile file-merge-2.txt modify merge 2

call :cmd_pysvn commit -m "change test merge files" .

call :cmd_pysvn merge --dry-run --revision 14:15 file:///b:/root/repos/trunk/test b:\wc3\test-branch
call :cmd_pysvn merge --revision 14:15 file:///b:/root/repos/trunk/test b:\wc3\test-branch
call :cmd_pysvn status b:\wc3\test-branch
call :cmd_pysvn diff b:\wc3\test-branch


call :cmd_shell %PYTHON% -u %BUILDER_TOP_DIR%\Tests\test_01_set_get_tests.py b:\configdir

echo Info: Test - import

call :cmd_shell mkdir b:\tmp
call :cmd_createfile b:\tmp\import1.txt import file 1
call :cmd_createfile "b:\tmp\import 2.txt" import file 2

call :cmd_pysvn mkdir "file:///b:/root/repos/trunk/test/import" -m "test-01 add import"

call :cmd_pysvn import --message "no spaces"    "b:\tmp\import1.txt" "file:///b:/root/repos/trunk/test/import/import-file1.txt"
call :cmd_pysvn import --message "space in url" "b:\tmp\import1.txt" "file:///b:/root/repos/trunk/test/import/import file1A.txt"
call :cmd_pysvn import --message "%20 in url"   "b:\tmp\import1.txt" "file:///b:/root/repos/trunk/test/import/import%20file1B.txt"

call :cmd_pysvn import --message "space in file, none in url"  "b:\tmp\import 2.txt" "file:///b:/root/repos/trunk/test/import/import-file2.txt"
call :cmd_pysvn import --message "space in file, space in url" "b:\tmp\import 2.txt" "file:///b:/root/repos/trunk/test/import/import file2A.txt"
call :cmd_pysvn import --message "space in file, %20 in url"   "b:\tmp\import 2.txt" "file:///b:/root/repos/trunk/test/import/import%20file2B.txt"

call :cmd_pysvn update b:\wc1
call :cmd_pysvn log --limit 6 --verbose b:\wc1

echo Info: Test - end
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
