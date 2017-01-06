#!/bin/bash
#
#   test-07.sh
#       test copy2, move2 and changelist
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

cmd mkdir testroot-07
cmd cd testroot-07

TESTROOT=${WORKDIR}/Tests/testroot-07

cmd mkdir tmp
export TMPDIR=${TESTROOT}/tmp

export PYTHONPATH=${WORKDIR}/Source:${WORKDIR}/Examples/Client
export PYSVN="${PYTHON} ${WORKDIR}/Examples/Client/svn_cmd.py --pysvn-testing 01.05.00 --config-dir ${TESTROOT}/configdir"
echo Info: PYSVN command ${PYSVN}

cmd svnadmin create ${TESTROOT}/repos

echo Info: Setup - mkdir
cmd_pysvn mkdir file://${TESTROOT}/repos/trunk -m "test-07 add trunk"
cmd_pysvn mkdir file://${TESTROOT}/repos/trunk/test -m "test-07 add test"

echo Info: Setup - checkout wc1
cmd_pysvn checkout file://${TESTROOT}/repos/trunk ${TESTROOT}/wc1
cmd cd ${TESTROOT}/wc1/test

echo Info: Setup - add files

echo test add file 1 >file_a1.txt
cmd_pysvn add file_a1.txt
cmd_pysvn checkin -m "commit added files"
echo test add file 2 >file_a2.txt
cmd_pysvn add file_a2.txt
cmd_pysvn checkin -m "commit added files"

echo test add file 1 >file_b1.txt
cmd_pysvn add file_b1.txt
cmd_pysvn checkin -m "commit added files"
echo test add file 2 >file_b2.txt
cmd_pysvn add file_b2.txt
cmd_pysvn checkin -m "commit added files"

cmd_pysvn status --verbose ${TESTROOT}/wc1

echo Info: running test_07_copy2
${PYTHON} ${WORKDIR}/Tests/test_07_copy2.py ${TESTROOT}/configdir

echo Info: running test_07_move2
${PYTHON} ${WORKDIR}/Tests/test_07_move2.py ${TESTROOT}/configdir

echo Info: running test_07_changelist
${PYTHON} ${WORKDIR}/Tests/test_07_changelist.py ${TESTROOT}/configdir

true
