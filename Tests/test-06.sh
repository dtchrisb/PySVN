#!/bin/bash
#
#   test-06.sh
#       test info and info2
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

cmd mkdir testroot-06
cmd cd testroot-06

TESTROOT=${WORKDIR}/Tests/testroot-06

cmd mkdir tmp
export TMPDIR=${TESTROOT}/tmp

export PYTHONPATH=${WORKDIR}/Source:${WORKDIR}/Examples/Client
export PYSVN="${PYTHON} ${WORKDIR}/Examples/Client/svn_cmd.py --pysvn-testing 01.03.00 --config-dir ${TESTROOT}/configdir"
echo Info: PYSVN command ${PYSVN}

cmd svnadmin create ${TESTROOT}/repos

echo Info: Setup - mkdir
cmd_pysvn mkdir file://${TESTROOT}/repos/trunk -m "test-06 add trunk"
cmd_pysvn mkdir file://${TESTROOT}/repos/trunk/test -m "test-06 add test"

echo Info: Setup - checkout wc1
cmd_pysvn checkout file://${TESTROOT}/repos/trunk ${TESTROOT}/wc1
cmd cd ${TESTROOT}/wc1/test

echo Info: Setup - add files
echo test add file 1 >file1.txt
echo test add file 2 >file2.txt
cmd_pysvn add file1.txt
cmd_pysvn add file2.txt
cmd_pysvn checkin -m "commit added files"

echo Info: Test - info of path
cmd_pysvn info file1.txt

echo Info: Test - info2 of path
cmd_pysvn info2 file1.txt

echo Info: Test - info2 of URL
cmd_pysvn info --revision HEAD file://${TESTROOT}/repos/trunk/test/file1.txt

echo Info: Test - info2 of URL
cmd_pysvn info2 --revision HEAD file://${TESTROOT}/repos/trunk/test/file1.txt

true
