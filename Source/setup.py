#
# ====================================================================
# (c) 2005-2009 Barry A Scott.  All rights reserved.
#
# This software is licensed as described in the file LICENSE.txt,
# which you should have received as part of this distribution.
#
# ====================================================================
#
#
#   setup.py
#
#   make it easy to build pysvn outside of svn
#
import sys
import os
import setup_backport

def main( argv ):
    if argv[1:2] == ['backport']:
        if setup_backport.backportRequired():
            return setup_backport.cmd_backport( argv )
        else:
            print( 'Info: These sources are compatible with python %d.%d - no need to run the backport command' %
                (sys.version_info[0], sys.version_info[1]) )
            return 0

    elif argv[1:2] == ['configure']:
        if setup_backport.backportRequired():
            print( 'Error: These sources are not compatible with python %d.%d - run the backport command to fix' %
                (sys.version_info[0], sys.version_info[1]) )
            return 1

        # must not import unless backporting has been done
        import setup_configure
        return setup_configure.cmd_configure( argv )

    elif argv[1:2] == ['help']:
        setup_help( argv )
        return 0

    else:
        return setup_help( argv )

def setup_help( argv ):
    progname = os.path.basename( argv[0] )
    print( '''    Help
        python %(progname)s help

    Backport the PySVN sources to work with python 2.5 and earlier

        python %(progname)s backport
''' % {'progname': progname} )

    if setup_backport.backportRequired():
        print( '    Further help is not available until the backport command has been run.' )
        return 1

    setup_backport.cmd_help( argv )

    import setup_configure
    setup_configure.cmd_help( argv )

    return 1

if __name__ == '__main__':
    sys.exit( main( sys.argv ) )
