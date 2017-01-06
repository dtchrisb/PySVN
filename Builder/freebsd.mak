build: all test kit

all:	../Source/Makefile
	cd ../Source && $(MAKE)

clean:	../Source/Makefile
	cd ../Source && $(MAKE) clean && rm Makefile
	cd ../Tests && $(MAKE) clean && rm Makefile

../Source/Makefile: ../Source/setup.py
	cd ../Source && $(PYTHON) setup.py configure
	

kit:
	cd ../Kit/FreeBSD && $(PYTHON) make_pkg.py

test:
	cd ../Tests && $(MAKE) all
