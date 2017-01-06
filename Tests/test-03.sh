#!/bin/bash
#
#   test-03.sh
#       test callbacks
#

# need to get rid of any symbolic links in the WORKDIR
export WORKDIR=$( ${PYTHON} -c 'import os;os.chdir("..");print( os.getcwd() )' )

cd ${WORKDIR}/Tests
echo WorkDir: ${WORKDIR}
echo PYTHON: ${PYTHON}

mkdir -p testroot-03
rm -rf testroot-03
mkdir testroot-03
cd testroot-03

mkdir configdir
${PYTHON} ../test_callbacks.py
