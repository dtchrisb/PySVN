#!/bin/bash
#
#   test-09.sh
#       test annotate, annotate2 and status2
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

cmd mkdir testroot-09
cmd cd testroot-09

TESTROOT=${WORKDIR}/Tests/testroot-09

cmd mkdir tmp
export TMPDIR=${TESTROOT}/tmp

export PYTHONPATH=${WORKDIR}/Source:${WORKDIR}/Examples/Client
export PYSVN="${PYTHON} ${WORKDIR}/Examples/Client/svn_cmd.py --pysvn-testing 01.07.00 --config-dir ${TESTROOT}/configdir"
echo Info: PYSVN command ${PYSVN}

cmd svnadmin create ${TESTROOT}/repos

echo Info: Setup - mkdir
cmd_pysvn mkdir file://${TESTROOT}/repos/trunk -m "test-09 add trunk"

echo Info: Setup - checkout wc1
cmd_pysvn checkout file://${TESTROOT}/repos/trunk ${TESTROOT}/wc1
cmd cd ${TESTROOT}/wc1

echo Info: Setup - add files and folders

cmd_pysvn mkdir folder1

echo test add file 1 >folder1/file-a.txt
cmd_pysvn add folder1/file-a.txt

cmd_pysvn checkin -m "commit added files"

echo test add line 2 >>folder1/file-a.txt
cmd_pysvn checkin -m "add line 2"

cmd_pysvn annotate folder1/file-a.txt

cmd_pysvn annotate2 folder1/file-a.txt

echo Info: propset_local
cmd_pysvn propset_local svn:eol native folder1/file-a.txt
cmd_pysvn proplist --verbose folder1/file-a.txt
cmd_pysvn checkin -m "eol is native"

cmd_pysvn propdel_local svn:eol folder1/file-a.txt
cmd_pysvn proplist --verbose folder1/file-a.txt
cmd_pysvn checkin -m "remove eol"

cmd_pysvn propset_remote test-case 09 file://${TESTROOT}/repos/trunk/folder1/file-a.txt --revision 5 <<EOF
set custom prop
EOF

cmd_pysvn proplist --verbose file://${TESTROOT}/repos/trunk/folder1/file-a.txt

cmd_pysvn propdel_remote test-case file://${TESTROOT}/repos/trunk/folder1/file-a.txt --revision 6 <<EOF
set custom prop
EOF

cmd_pysvn proplist --verbose file://${TESTROOT}/repos/trunk/folder1/file-a.txt

cmd_pysvn update

cmd cd folder1
cmd touch unversioned.txt
cmd_pysvn status2 
cmd_pysvn status2 --verbose
cmd_pysvn status2 --verbose --quiet

cmd_pysvn lock file-a.txt
cmd_pysvn status2 
cmd_pysvn status2 --verbose
cmd_pysvn status2 --verbose --quiet

true
