print( 'Info: test_01_set_get_tests start' )
import sys
print( 'Info: test_01_set_get_tests import pysvn' )
import pysvn

print( 'Info: test_01_set_get_tests pysvn.Client( %s )' % (sys.argv[1],) )
c = pysvn.Client( sys.argv[1] )

print( 'Info: Initial values' )

print( 'Info: get_auth_cache() => %r' % c.get_auth_cache() )
print( 'Info: get_auto_props() => %r' % c.get_auto_props() )
print( 'Info: get_default_password() => %r' % c.get_default_password() )
print( 'Info: get_default_username() => %r' % c.get_default_username() )
print( 'Info: get_interactive() => %r' % c.get_interactive() )
print( 'Info: get_store_passwords() => %r' % c.get_store_passwords() )

print( 'Info: Change values 1' )

c.set_auth_cache( False )
c.set_auto_props( False )
c.set_default_password( "thepass" )
c.set_default_username( "auser" )
c.set_interactive( False )
c.set_store_passwords( False )

print( 'Info: Changed values 1' )

print( 'Info: get_auth_cache() => %r' % c.get_auth_cache() )
print( 'Info: get_auto_props() => %r' % c.get_auto_props() )
print( 'Info: get_default_password() => %r' % c.get_default_password() )
print( 'Info: get_default_username() => %r' % c.get_default_username() )
print( 'Info: get_interactive() => %r' % c.get_interactive() )
print( 'Info: get_store_passwords() => %r' % c.get_store_passwords() )

print( 'Info: Change values 2' )

c.set_auth_cache( True )
c.set_auto_props( True )
c.set_default_password( None )
c.set_default_username( None )
c.set_interactive( True )
c.set_store_passwords( True )

print( 'Info: Changed values 2' )

print( 'Info: get_auth_cache() => %r' % c.get_auth_cache() )
print( 'Info: get_auto_props() => %r' % c.get_auto_props() )
print( 'Info: get_default_password() => %r' % c.get_default_password() )
print( 'Info: get_default_username() => %r' % c.get_default_username() )
print( 'Info: get_interactive() => %r' % c.get_interactive() )
print( 'Info: get_store_passwords() => %r' % c.get_store_passwords() )

print( 'Info: test_01_set_get_tests dealloc Client()' )
del c
print( 'Info: test_01_set_get_tests done' )
