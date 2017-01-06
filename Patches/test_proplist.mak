CCCFLAGS=-fPIC -fexceptions -frtti -I$(SVN_INC) -I$(APR_INC)
LDLIBS=-L$(SVN_LIB) -lsvn_client-1 -lapr-0

test_proplist: test_proplist.o
	g++ -g -o test_proplist test_proplist.o $(LDLIBS)

test_proplist.o: test_proplist.cpp
	g++ -c -g $(CCCFLAGS) -o $@ $<
