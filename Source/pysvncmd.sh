#!/bin/bash -x
PY_VER=${1:?py-ver}
SVN_VER=${2:?svn-ver}
shift
shift

case "$(uname -s)" in
Darwin)
    export LD_LIBRARY_PATH=/usr/local/svn-${SVN_VER}/lib:/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.11.sdk/usr/lib
    ;;
*)
    ;;
esac

export PYTHONPATH=${BUILDER_TOP_DIR}/Source:${BUILDER_TOP_DIR}/Examples/Client
python${PY_VER} ${BUILDER_TOP_DIR}/Examples/Client/svn_cmd.py "$@"
