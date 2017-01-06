#!/bin/bash
#
#   test-01.sh
#       test the main commands that are in all versions
#

# need to get rid of any symbolic links in the WORKDIR
export WORKDIR=$( ${PYTHON} -c 'import os;os.chdir("..");print( os.getcwd() )' )

cd ${WORKDIR}/Tests
echo WorkDir: ${WORKDIR}
echo PYTHON: ${PYTHON}
echo Username: $(id -u -n)

cmd () {
        echo Info: CWD: $(pwd)
	echo Info: Command: $*
	"$@"
}

cmd_pysvn () {
        echo Info: CWD: $(pwd)
	echo Info: pysvn command: $*
	${PYSVN} "$@"
}


cmd mkdir testroot-01
cmd cd testroot-01

TESTROOT=${WORKDIR}/Tests/testroot-01

cmd mkdir tmp
export TMPDIR=${TESTROOT}/tmp

export PYTHONPATH=${WORKDIR}/Source:${WORKDIR}/Examples/Client
export PYSVN="${PYTHON} ${WORKDIR}/Examples/Client/svn_cmd.py --pysvn-testing 01.01.00 --config-dir ${TESTROOT}/configdir"
echo Info: PYSVN command ${PYSVN}

cmd svnadmin create ${TESTROOT}/repos

echo Info: Testing - mkdir
cmd_pysvn mkdir file://${TESTROOT}/repos/trunk -m "test-01 add trunk"
cmd_pysvn mkdir file://${TESTROOT}/repos/trunk/test -m "test-01 add test"

echo Info: Testing - ls
cmd_pysvn ls file://${TESTROOT}/repos -v -R

echo Info: Testing - checkout
cmd_pysvn checkout file://${TESTROOT}/repos/trunk ${TESTROOT}/wc1
cmd ${PYTHON} ${WORKDIR}/Tests/find.py ${TESTROOT}/wc1
cmd cd ${TESTROOT}/wc1/test

echo Info: Testing - add
echo test add file 1 >file1.txt
echo test add file 2 >file2.txt
echo test add file 3 >file3.txt
echo test add file 4 >file4.txt
echo test add file 5 >file5.txt
echo test add file 6 >file6.txt
mkdir folder1
echo test add file 7 >folder1/file7.txt
mkdir folder1/folder2
echo test add file 8 >folder1/folder2/file8.txt
mkdir folder3
echo test add file 9 >folder3/file9.txt
mkdir folder3/folder4
echo test add file 10 >folder3/folder4/file10.txt

cmd_pysvn add file1.txt
cmd_pysvn add file2.txt
cmd_pysvn add file3.txt
cmd_pysvn add file4.txt
cmd_pysvn add --force file5.txt
cmd_pysvn add file6.txt

cmd_pysvn add folder1
cmd_pysvn add --non-recursive folder3

cmd_pysvn checkin -m "commit added files"

echo Info: Setup to test access to deleted files
echo test mod file 6 >>file6.txt
cmd_pysvn checkin -m "commit mod file"
cmd_pysvn rm file6.txt
cmd_pysvn checkin -m "commit delete file"

echo Info: Testing - update - get a new wc that will update
cmd_pysvn checkout file://${TESTROOT}/repos/trunk ${TESTROOT}/wc2

echo Info: Testing - - checkin a mod from wc1
echo line 2 >>${TESTROOT}/wc1/test/file1.txt
cmd_pysvn checkin -m "commit modified file"

echo Info: Testing - update
cmd_pysvn update ${TESTROOT}/wc2

echo Info: Testing - the rest in lexical order

echo Info: Testing - annotate
cmd_pysvn annotate ${TESTROOT}/wc2/test/file1.txt
cmd_pysvn annotate -r 3:4 file://${TESTROOT}/repos/trunk/test/file6.txt

echo Info: Testing - cat
cmd_pysvn cat -r head file://${TESTROOT}/repos/trunk/test/file1.txt
cmd_pysvn cat -r 4 file://${TESTROOT}/repos/trunk/test/file6.txt

echo Info: Testing - cleanup
cmd_pysvn cleanup ${TESTROOT}/wc1
cmd cd ${TESTROOT}/wc1
cmd_pysvn cleanup .
cmd cd ${TESTROOT}/wc1/test

echo Info: Testing - copy
cmd_pysvn mkdir file://${TESTROOT}/repos/tags -m "test-01 add tags"
echo tag the trunk >msg.tmp
cmd_pysvn copy file://${TESTROOT}/repos/trunk file://${TESTROOT}/repos/tags/version1 <msg.tmp
rm msg.tmp
cmd_pysvn ls -v file://${TESTROOT}/repos/tags
cmd_pysvn copy ${TESTROOT}/wc2/test/file1.txt ${TESTROOT}/wc2/test/file1b.txt
cmd_pysvn propset svn:eol-style native ${TESTROOT}/wc2/test/file1b.txt
cmd_pysvn checkin ${TESTROOT}/wc2 -m "copy test"

echo Info: Testing - diff
echo new line >>${TESTROOT}/wc2/test/file1b.txt
cmd_pysvn diff ${TESTROOT}/wc2

echo Info: Testing - export
cmd_pysvn export file://${TESTROOT}/repos/trunk/test ${TESTROOT}/export1.native
cmd_pysvn export --native-eol CR file://${TESTROOT}/repos/trunk/test ${TESTROOT}/export1.cr
cmd_pysvn export --native-eol LF file://${TESTROOT}/repos/trunk/test ${TESTROOT}/export1.lf
cmd_pysvn export --native-eol CRLF file://${TESTROOT}/repos/trunk/test ${TESTROOT}/export1.crlf
${PYTHON} ${WORKDIR}/Tests/find.py ${TESTROOT}/export1.native
${PYTHON} ${WORKDIR}/Tests/find.py ${TESTROOT}/export1.cr
${PYTHON} ${WORKDIR}/Tests/find.py ${TESTROOT}/export1.lf
${PYTHON} ${WORKDIR}/Tests/find.py ${TESTROOT}/export1.crlf

echo Info: Testing - info
cmd_pysvn info ${TESTROOT}/wc2/test
cmd_pysvn info ${TESTROOT}/wc2/test/file1.txt

echo Info: Testing - log
cmd_pysvn log ${TESTROOT}/wc2

echo Info: Testing - ls
cmd_pysvn ls file://${TESTROOT}/repos/trunk/test
cmd_pysvn ls -v file://${TESTROOT}/repos/trunk/test
cmd_pysvn ls ${TESTROOT}/wc2/test
cmd_pysvn ls -v ${TESTROOT}/wc2/test

echo Info: Testing - merge - done below
echo Info: Testing - mkdir - done above

echo Info: Testing - move
echo move url test >msg.tmp
cmd_pysvn move file://${TESTROOT}/repos/trunk/test/file2.txt file://${TESTROOT}/repos/trunk/test/file2b.txt <msg.tmp
rm msg.tmp
cmd_pysvn move ${TESTROOT}/wc2/test/file3.txt ${TESTROOT}/wc2/test/file3b.txt
cmd_pysvn checkin ${TESTROOT}/wc2 -m "move wc test"

echo Info: Testing - status
echo file 4 is changing >>${TESTROOT}/wc1/test/file4.txt
cmd_pysvn checkin ${TESTROOT}/wc1 -m "change wc1 for status -u to detect"

cmd_pysvn status ${TESTROOT}/wc2
cmd_pysvn status --verbose ${TESTROOT}/wc2
cmd_pysvn status --show-updates ${TESTROOT}/wc2
cmd_pysvn status --show-updates --verbose ${TESTROOT}/wc2
cmd_pysvn update
cmd_pysvn status --show-updates ${TESTROOT}/wc2
cmd_pysvn status --show-updates --verbose ${TESTROOT}/wc2
cmd_pysvn checkin ${TESTROOT}/wc2 -m "prop change"

echo Info: Testing - propdel
cd ${TESTROOT}/wc2/test
cmd_pysvn propset test:prop1 del_me file4.txt
cmd_pysvn proplist -v file4.txt
cmd_pysvn propdel test:prop1 file4.txt
cmd_pysvn proplist -v file4.txt

echo Info: Testing - propget
cmd_pysvn propset svn:eol-style native file4.txt
cmd_pysvn propget svn:eol-style file4.txt
cmd_pysvn propget unknown file4.txt

echo Info: Testing - proplist - see above

echo Info: Testing - propset
cd ${TESTROOT}/wc2/test
cmd_pysvn proplist -v file4.txt
cmd_pysvn propset svn:eol-style native file4.txt
cmd_pysvn proplist -v file4.txt

echo Info: Testing - remove
cd ${TESTROOT}/wc2/test
cmd_pysvn remove file5.txt
cmd_pysvn status

echo Info: Testing - resolved
echo conflict in file4 yes >>${TESTROOT}/wc1/test/file4.txt
echo conflict in file4 no >>${TESTROOT}/wc2/test/file4.txt
cmd_pysvn checkin ${TESTROOT}/wc1/test -m "make a conflict part 1"
cmd_pysvn update ${TESTROOT}/wc2/test
cmd_pysvn status
cmd cp ${TESTROOT}/wc2/test/file4.txt.mine ${TESTROOT}/wc2/test/file4.txt
cmd_pysvn resolved ${TESTROOT}/wc2/test/file4.txt
cmd_pysvn checkin ${TESTROOT}/wc2/test/file4.txt -m "resolve a confict part 2"

echo Info: Testing - revert
cmd_pysvn revert file5.txt
cmd_pysvn status

echo Info: Testing - revproplist
cmd_pysvn revproplist file://${TESTROOT}/repos/trunk

echo Info: Testing - revpropget
cmd_pysvn revpropget svn:log file://${TESTROOT}/repos/trunk
cmd_pysvn revpropget no_such_prop file://${TESTROOT}/repos/trunk

echo Info: Testing - revpropset
cmd_pysvn revpropset svn:log "Hello world" file://${TESTROOT}/repos/trunk

echo Info: Testing - revpropdel
cmd_pysvn revpropdel svn:log file://${TESTROOT}/repos/trunk

echo Info: Testing - status - see above

echo Info: Testing - relocate
cmd mkdir ${TESTROOT}/root
cmd mv ${TESTROOT}/repos ${TESTROOT}/root
cmd_pysvn info ${TESTROOT}/wc1
cmd_pysvn relocate file://${TESTROOT}/repos/trunk file://${TESTROOT}/root/repos/trunk ${TESTROOT}/wc1
cmd_pysvn info ${TESTROOT}/wc1
cmd_pysvn info ${TESTROOT}/wc2
cmd_pysvn relocate file://${TESTROOT}/repos/trunk file://${TESTROOT}/root/repos/trunk ${TESTROOT}/wc2
cmd_pysvn info ${TESTROOT}/wc2

echo Info: Testing - switch
cmd_pysvn info ${TESTROOT}/wc2
cmd_pysvn switch ${TESTROOT}/wc2 file://${TESTROOT}/root/repos/tags/version1
cmd_pysvn info ${TESTROOT}/wc2

echo Info: Testing - update - see above

echo Info: Testing - merge
cmd_pysvn checkout file://${TESTROOT}/root/repos/trunk ${TESTROOT}/wc3
cmd cd ${TESTROOT}/wc3/test
echo test add file merge 1 >file-merge-1.txt
echo test add file merge 2 >file-merge-2.txt
cmd_pysvn add file-merge-1.txt
cmd_pysvn add file-merge-2.txt
cmd_pysvn commit -m "add test merge files" .

echo make a branch >msg.tmp
cmd_pysvn copy file://${TESTROOT}/root/repos/trunk/test file://${TESTROOT}/root/repos/trunk/test-branch <msg.tmp
cmd_pysvn update ${TESTROOT}/wc3

echo test add file merge 3 >file-merge-3.txt
cmd_pysvn add file-merge-3.txt
cmd_pysvn rm file-merge-1.txt
echo modify merge 2 >>file-merge-2.txt

cmd_pysvn commit -m "change test merge files" .

cmd_pysvn merge --dry-run --revision 16:17 file://${TESTROOT}/root/repos/trunk/test ${TESTROOT}/wc3/test-branch
cmd_pysvn merge --revision 16:17 file://${TESTROOT}/root/repos/trunk/test ${TESTROOT}/wc3/test-branch
cmd_pysvn status ${TESTROOT}/wc3/test-branch
cmd_pysvn diff ${TESTROOT}/wc3/test-branch

cmd ${PYTHON} ${WORKDIR}/Tests/test_01_set_get_tests.py ${TESTROOT}/configdir

echo Info: Testing - import
echo test import file 1 >"${TMPDIR}/import1.txt"
echo test import file 2 >"${TMPDIR}/import 2.txt"

cmd_pysvn mkdir "file://${TESTROOT}/root/repos/trunk/test/import" -m "test-01 add import"

cmd_pysvn import --message "no spaces"    "${TMPDIR}/import1.txt" "file://${TESTROOT}/root/repos/trunk/test/import/import-file1.txt"
cmd_pysvn import --message "space in url" "${TMPDIR}/import1.txt" "file://${TESTROOT}/root/repos/trunk/test/import/import file1A.txt"
cmd_pysvn import --message "%20 in url"   "${TMPDIR}/import1.txt" "file://${TESTROOT}/root/repos/trunk/test/import/import%20file1B.txt"

cmd_pysvn import --message "space in file, none in url"  "${TMPDIR}/import 2.txt" "file://${TESTROOT}/root/repos/trunk/test/import/import-file2.txt"
cmd_pysvn import --message "space in file, space in url" "${TMPDIR}/import 2.txt" "file://${TESTROOT}/root/repos/trunk/test/import/import file2A.txt"
cmd_pysvn import --message "space in file, %20 in url"   "${TMPDIR}/import 2.txt" "file://${TESTROOT}/root/repos/trunk/test/import/import%20file2B.txt"

cmd_pysvn update ${TESTROOT}/wc1
cmd_pysvn log --limit 6 --verbose ${TESTROOT}/wc1
