src_dir='../../Source'
import os
if not os.path.exists( os.path.join( src_dir, 'pysvn/__init__.py' ) ):
    raise RuntimeError('Where is the pysvn module? pwd=%s' % os.getcwd() )

import sys
sys.path.insert( 0, src_dir )
import pysvn

class Test:
    def __init__( self ):
        self.pass_count = 0
        self.fail_count = 0

    def test( self ):
        self.test_1()

        print( 'Info: Passed %d' % self.pass_count )
        if self.fail_count > 0:
            print( 'Info: FAILED %d' % self.pass_count )

        return self.fail_count == 0

    def info( self, msg ):
        print( 'Info: %s' % msg )

    def passed( self ):
        print( 'Info: passed' )
        self.pass_count += 1

    def failed( self, msg ):
        print( 'Error: FAILED %s' % msg )
        self.fail_count += 1

    def test_1( self ):

        self.client = pysvn.Client( 'configdir' )
        self.info( 'Client created' )

        self.test_1_sub1( 'callback_get_login required' )

        self.client.callback_get_login = get_login_bad
        self.test_1_sub1( 'unhandled exception in callback_get_login' )

        self.client.callback_get_login = get_login_good
        self.test_1_sub1( 'callback_get_log_message required' )

        self.client.callback_get_log_message = get_log_message_bad
        self.test_1_sub1( 'unhandled exception in callback_get_log_message' )

        self.client.callback_get_log_message = get_log_message_good
        self.test_1_sub1()

        self.client.remove( 'http://liara/svn/barrys-test-lib/trunk/fred/testing/bar99.txt' )

    def test_1_sub1( self, expected=None ):
        try:
            self.info( 'Expecting error %s' % expected )
            self.client.copy(
                'http://liara/svn/barrys-test-lib/trunk/fred/testing/bar.txt',
                'http://liara/svn/barrys-test-lib/trunk/fred/testing/bar99.txt' )
        except pysvn.ClientError as e:
            if expected is None:
                self.failed( 'unexpected exception: %s' % e )

            if str(e) == expected:
                self.passed()
            else:
                self.failed( 'unexpected exception: %s' % e )
            return

        if expected is not None:
            self.failed( 'expected exception' )



def get_login_bad( realm, username, may_save ):
    # bad because of undefined retcode
    return retcode, username, password, save

def get_login_good( realm, username, may_save ):
    return True, username, 'fred', may_save

def get_log_message_bad():
    # bad because of undefined bad_var
    return bad_var

def get_log_message_good():
    return True, 'test_03 reason'


if __name__ == '__main__':
    t = Test()
    if t.test():
        sys.exit( 0 )
    else:
        sys.exit( 1 )
