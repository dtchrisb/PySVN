import sys
import os
import threading
import pysvn
import time

def main( args ):
	num_threads = int(args[0])
	repo = args[1]

	all_threads = []
	for index in range(num_threads):
		t = CheckOutThread( index, repo )
		all_threads.append( t )
		t.start()

	some_alive = True
	while some_alive:
		some_alive = False
		time.sleep( 1 )
		for t in all_threads:
			if t.isAlive():
				some_alive = True
	print 'Done'

class CheckOutThread(threading.Thread):
	def __init__( self, index, repo ):
		threading.Thread.__init__( self )
		self.co_dir = os.path.join( 'testroot', 'wc_%d' % index )
		self.co_repo = repo

	def run( self ):
		print 'Checkout',self.co_repo,self.co_dir
		client = pysvn.Client()
		client.callback_notify = self.callback_notify
		client.callback_cancel = self.callback_cancel
		x = client.checkout( self.co_repo, self.co_dir )
		print 'Checkout to',self.co_dir,'Done'

	def callback_cancel( self ):
		print self.co_dir, 'Cancel check'
		return False

	def callback_notify( self, arg_dict ):
		print self.co_dir, arg_dict['action'], arg_dict['path']

if __name__ == '__main__':
	main( sys.argv[1:] )
