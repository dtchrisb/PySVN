import pysvn
import sys

c = pysvn.Client( sys.argv[1] )
c.exception_style = 1

src_list = [
    'file_a1.txt',
    'file_a2.txt',
    ]
print( 'Info: Move2 will succeed' )
c.move2( src_list, 'wc_move_1', make_parents=True, move_as_child=True )

src_list = [
    'file_b1.txt',
    'file_b2.txt',
    ]

print( 'Info: Move2 will raise error' )
try:
    c.move2( src_list, 'wc_move_3', make_parents=True )

except pysvn.ClientError as e:
    print( 'Info: Error: %r' % e.args[0] )
    for message, code in e.args[1]:
        print( 'Info: Code: %d, Message: %r' % (code, message) )
