#!/bin/bash
#
#   test-05.sh
#       test info2 with locked files
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

cmd mkdir testroot-05
cmd cd testroot-05

TESTROOT=${WORKDIR}/Tests/testroot-05

cmd mkdir tmp
export TMPDIR=${TESTROOT}/tmp

export PYTHONPATH=${WORKDIR}/Source:${WORKDIR}/Examples/Client
export PYSVN="${PYTHON} ${WORKDIR}/Examples/Client/svn_cmd.py --pysvn-testing 01.02.01 --config-dir ${TESTROOT}/configdir"
echo Info: PYSVN command ${PYSVN}

cmd svnadmin create ${TESTROOT}/repos

echo Info: Setup - mkdir
cmd_pysvn mkdir file://${TESTROOT}/repos/trunk -m "test-05 add trunk"
cmd_pysvn mkdir file://${TESTROOT}/repos/trunk/test -m "test-05 add test"

echo Info: Setup - checkout wc1
cmd_pysvn checkout file://${TESTROOT}/repos/trunk ${TESTROOT}/wc1
cmd cd ${TESTROOT}/wc1/test

echo Info: Setup - add files
echo test add file 1 >file1.txt
echo test add file 2 >file2.txt
cmd_pysvn add file1.txt
cmd_pysvn add file2.txt
cmd_pysvn checkin -m "commit added files"

echo Info: Setup - checkout wc2
cmd_pysvn checkout file://${TESTROOT}/repos/trunk ${TESTROOT}/wc2

echo Info: Test - status of unlocked files
cmd_pysvn status --verbose ${TESTROOT}/wc1

echo Info: Test - info2 of unlocked files
cmd_pysvn info2 ${TESTROOT}/wc1/test/file1.txt

echo Info: Test - list of unlocked files
cmd_pysvn list --verbose --fetch-locks ${TESTROOT}/wc1/test/file1.txt
cmd_pysvn list --verbose --fetch-locks ${TESTROOT}/wc1/test

echo Info: Test - lock unlocked file
cmd_pysvn lock ${TESTROOT}/wc1/test/file1.txt -m "lock comment test 05"

echo Info: Test - status of locked files
cmd_pysvn status --verbose ${TESTROOT}/wc1

echo Info: Test - info2 of locked files
cmd_pysvn info2 ${TESTROOT}/wc1/test/file1.txt

echo Info: Test - list of locked files
cmd_pysvn list --verbose --fetch-locks ${TESTROOT}/wc1/test/file1.txt
cmd_pysvn list --verbose --fetch-locks ${TESTROOT}/wc1/test

echo Info: Test - attempt to checkin over a locked file
cmd cd ${TESTROOT}/wc2/test
echo Change to file 1 >>file1.txt
echo Change to file 2 >>file2.txt
cmd_pysvn commit -m "change when file locked in other wc" .

echo Info: Test - lock locked file
cmd_pysvn lock ${TESTROOT}/wc2/test/file1.txt

echo Info: Test - lock --force locked file
cmd_pysvn lock --force ${TESTROOT}/wc2/test/file1.txt -m "Stealing lock"

echo Info: Test - info2 of locked files
cmd_pysvn info2 ${TESTROOT}/wc2/test/file1.txt

echo Info: Test - status of locked files
cmd_pysvn status --verbose ${TESTROOT}/wc2

echo Info: Test - commit with lock
cmd_pysvn commit -m "change when file locked in this wc" .

echo Info: Test - status of locked files
cmd_pysvn status --verbose ${TESTROOT}/wc2

echo Info: Test - list of locked files
cmd_pysvn list --verbose --fetch-locks --recursive ${TESTROOT}/wc2

echo Info: Test - unlock locked file
cmd_pysvn unlock ${TESTROOT}/wc2/test/file1.txt

echo Info: Test - status of unlocked files
cmd_pysvn status --verbose ${TESTROOT}/wc2

echo Info: Test - list of unlocked files
cmd_pysvn list --verbose --fetch-locks --recursive ${TESTROOT}/wc2

echo Info: Test - status of locked files
cmd_pysvn status --verbose ${TESTROOT}/wc1

echo Info: Test - list of unlocked files
cmd_pysvn list --verbose --fetch-locks --recursive ${TESTROOT}/wc1

echo Info: Test - update with stolen lock
cmd_pysvn update ${TESTROOT}/wc1/test

echo Info: Test - status of locked files
cmd_pysvn status --verbose ${TESTROOT}/wc1

echo Info: Test - info2 of URL
cmd_pysvn info2 --revision HEAD file://${TESTROOT}/repos/trunk/test/file1.txt

echo Info: Test - list of locked files
cmd_pysvn list --verbose --fetch-locks --recursive ${TESTROOT}/wc1

true
