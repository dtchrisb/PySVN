#!/bin/bash
#
#   test-10.sh - 1.9 commands
#       test vacuum
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

cmd mkdir testroot-10
cmd cd testroot-10

TESTROOT=${WORKDIR}/Tests/testroot-10

cmd mkdir tmp
export TMPDIR=${TESTROOT}/tmp

export PYTHONPATH=${WORKDIR}/Source:${WORKDIR}/Examples/Client
export PYSVN="${PYTHON} ${WORKDIR}/Examples/Client/svn_cmd.py --pysvn-testing 01.09.00 --config-dir ${TESTROOT}/configdir"
echo Info: PYSVN command ${PYSVN}

cmd svnadmin create ${TESTROOT}/repos

echo Info: Setup - mkdir
cmd_pysvn mkdir file://${TESTROOT}/repos/trunk -m "test-10 add trunk"

echo Info: Setup - checkout wc1
cmd_pysvn checkout file://${TESTROOT}/repos/trunk ${TESTROOT}/wc1
cmd cd ${TESTROOT}/wc1

echo Info: Setup - add files and folders

cmd_pysvn mkdir folder1
cmd_pysvn propset svn:ignore "*~" folder1

echo test add file 1 >folder1/file-a.txt
cmd_pysvn add folder1/file-a.txt

cmd_pysvn checkin -m "commit added files"

echo test add file 1 >folder1/file-a.txt~
echo test add file 1 >folder1/file-b.txt

echo Info: vacuum no removes
cmd_pysvn status2 --verbose --no-ignore .
cmd_pysvn vacuum

echo Info: vacuum remove ignored
cmd_pysvn status2 --verbose --no-ignore .
cmd_pysvn vacuum --remove-ignored-items

echo Info: vacuum remove versioned
cmd_pysvn status2 --verbose --no-ignore .
cmd_pysvn vacuum --remove-unversioned-items

echo Info: check final state
cmd_pysvn status2 --verbose --no-ignore .

true
