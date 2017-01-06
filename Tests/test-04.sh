#!/bin/bash
#
#   test-04.sh
#       test Tranaction object
#

# need to get rid of any symbolic links in the WORKDIR
export WORKDIR=$( ${PYTHON} -c 'import os;os.chdir("..");print( os.getcwd() )' )

cd ${WORKDIR}/Tests
echo WorkDir: ${WORKDIR}
echo PYTHON: ${PYTHON}
echo Username: $(id -u -n)

cmd () {
	echo Info: Command: $*
	"$@"
}

cmd mkdir testroot-04
cmd cd testroot-04

TESTROOT=${WORKDIR}/Tests/testroot-04

cmd mkdir tmp
export TMPDIR=${TESTROOT}/tmp

export PYTHONPATH=${WORKDIR}/Source:${WORKDIR}/Examples/Client
export PYSVN="${PYTHON} ${WORKDIR}/Examples/Client/svn_cmd.py --pysvn-testing 01.01.00 --config-dir ${TESTROOT}/configdir"

cmd svnadmin create ${TESTROOT}/repos

echo Info: Testing - mkdir
cmd ${PYSVN} mkdir file://${TESTROOT}/repos/trunk -m "test-04 add trunk"
cmd ${PYSVN} mkdir file://${TESTROOT}/repos/trunk/test -m "test-04 add test"

echo Info: Install hooks
echo '#!/bin/bash' >${TESTROOT}/repos/hooks/pre-commit
echo export PYTHONPATH=$PYTHONPATH >>${TESTROOT}/repos/hooks/pre-commit
echo export LD_LIBRARY_PATH=$LD_LIBRARY_PATH >>${TESTROOT}/repos/hooks/pre-commit
echo export PATH=$PATH >>${TESTROOT}/repos/hooks/pre-commit
echo echo $PYTHON ${WORKDIR}/Tests/test_04_commit_hook_test_1.py pre-commit '"$@"' ">${TESTROOT}/pre_test_1.output" >>${TESTROOT}/repos/hooks/pre-commit
echo $PYTHON ${WORKDIR}/Tests/test_04_commit_hook_test_1.py pre-commit '"$@"' ">>${TESTROOT}/pre_test_1.output" >>${TESTROOT}/repos/hooks/pre-commit
chmod +x ${TESTROOT}/repos/hooks/pre-commit

echo '#!/bin/bash' >${TESTROOT}/repos/hooks/post-commit
echo export PYTHONPATH=$PYTHONPATH >>${TESTROOT}/repos/hooks/post-commit
echo export LD_LIBRARY_PATH=$LD_LIBRARY_PATH >>${TESTROOT}/repos/hooks/post-commit
echo export PATH=$PATH >>${TESTROOT}/repos/hooks/post-commit
echo echo $PYTHON ${WORKDIR}/Tests/test_04_commit_hook_test_1.py post-commit '"$@"' is_revision ">${TESTROOT}/post_test_1.output" >>${TESTROOT}/repos/hooks/post-commit
echo $PYTHON ${WORKDIR}/Tests/test_04_commit_hook_test_1.py post-commit '"$@"' is_revision ">>${TESTROOT}/post_test_1.output" >>${TESTROOT}/repos/hooks/post-commit
chmod +x ${TESTROOT}/repos/hooks/post-commit

cmd ${PYSVN} mkdir file://${TESTROOT}/repos/trunk/test/a -m "pre-commit test 1"
echo Info: pre_test_1.output start ------------------------------------
cat ${TESTROOT}/pre_test_1.output
echo Info: pre_test_1.output end --------------------------------------
echo Info: post_test_1.output start -----------------------------------
cat ${TESTROOT}/post_test_1.output
echo Info: post_test_1.output end -------------------------------------


echo Info: Add two files
cmd ${PYSVN} checkout file://${TESTROOT}/repos/trunk/test ${TESTROOT}/wc

echo file1 ROOT > ${TESTROOT}/wc/file1.txt
echo file1 A > ${TESTROOT}/wc/a/file1.txt

cmd ${PYSVN} add ${TESTROOT}/wc/file1.txt
cmd ${PYSVN} add ${TESTROOT}/wc/a/file1.txt
cmd ${PYSVN} checkin -m "Add two files" ${TESTROOT}/wc
echo Info: pre_test_1.output start ------------------------------------
cat ${TESTROOT}/pre_test_1.output
echo Info: pre_test_1.output end --------------------------------------
echo Info: post_test_1.output start -----------------------------------
cat ${TESTROOT}/post_test_1.output
echo Info: post_test_1.output end -------------------------------------

echo Info: Mod one file Mod one prop

echo file1 ROOT ln 2 > ${TESTROOT}/wc/file1.txt
cmd ${PYSVN} propset svn:eol-style native ${TESTROOT}/wc/a/file1.txt
cmd ${PYSVN} checkin -m "Mod one file Mod one prop" ${TESTROOT}/wc
echo Info: pre_test_1.output start ------------------------------------
cat ${TESTROOT}/pre_test_1.output
echo Info: pre_test_1.output end --------------------------------------
echo Info: post_test_1.output start -----------------------------------
cat ${TESTROOT}/post_test_1.output
echo Info: post_test_1.output end -------------------------------------

echo Info: Delete one file

cmd ${PYSVN} rm ${TESTROOT}/wc/a/file1.txt
cmd ${PYSVN} checkin -m "Delete one file" ${TESTROOT}/wc
echo Info: pre_test_1.output start ------------------------------------
cat ${TESTROOT}/pre_test_1.output
echo Info: pre_test_1.output end --------------------------------------
echo Info: post_test_1.output start -----------------------------------
cat ${TESTROOT}/post_test_1.output
echo Info: post_test_1.output end -------------------------------------

echo Info: Copy one file

cmd ${PYSVN} cp ${TESTROOT}/wc/file1.txt ${TESTROOT}/wc/file1copy.txt
cmd ${PYSVN} checkin -m "Copy one file" ${TESTROOT}/wc
echo Info: pre_test_1.output start ------------------------------------
cat ${TESTROOT}/pre_test_1.output
echo Info: pre_test_1.output end --------------------------------------
echo Info: post_test_1.output start -----------------------------------
cat ${TESTROOT}/post_test_1.output
echo Info: post_test_1.output end -------------------------------------
