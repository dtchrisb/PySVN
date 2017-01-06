import pysvn
import sys

def main(argv):
    print( 'Info: Checking for min version: %s against SVN version %s' % ('.'.join(argv[1:]), pysvn.svn_version) )
    for index in range(1,len(argv)):
        if pysvn.svn_version[ index-1 ] > int( argv[ index ] ):
            break
        if pysvn.svn_version[ index-1 ] < int( argv[ index ] ):
            print( 'Warning: SVN version is not new enough' )
            return 1

    print( 'Info: SVN version is new enough' )
    return 0

if __name__ == '__main__':
    sys.exit( main( sys.argv ) )
