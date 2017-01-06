#!/bin/bash

export SVN_INC=/usr/include/subversion-1
export SVN_LIB=/usr/lib
if [ -e /usr/include/apr-0 ]
then
    export APR_INC=/usr/include/apr-0
else
    export APR_INC=/usr/include/apache2
fi
export APR_LIB=/usr/lib

make -f test_proplist.mak
