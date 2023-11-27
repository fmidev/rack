

CC=g++
DRAIN=../..
H5ROOT=/usr/include/hdf5/serial
#LIBS=-ldrain -lhdf5
CCFLAGS=-I.. -I/usr/include/hdf5/serial
LDFLAGS=-ldrain -L${DRAIN}/drain/Debug/ -lhdf5  -L/usr/lib/x86_64-linux-gnu/serial/

Quantity-example: Quantity-example.cpp  
	g++ ${CCFLAGS} $< ../data/{EncodingODIM,ODIMPath}.cpp  -L${DRAIN}/drain/Debug -ldrain   -o $@


Hi5Read-example: Hi5Read-example.cpp ../hi5 
	g++ ${CCFLAGS} $< ../hi5/Hi5.cpp ../hi5/Hi5Read.cpp  ${LDFLAGS} -o $@

ODIM-example: ODIM-example.cpp ../data/ODIM.h 
	g++ ${CCFLAGS} $<  ../data/ODIM.cpp ${LDFLAGS}  -o $@


Command-example: Command-example.cpp ../main/products.h ../main/commands.h 
	g++ ${CCFLAGS} $< ../data/ODIM.cpp  ../main/resources.cpp ../data/DataSelector.cpp ../main/commands.cpp  ${LDFLAGS}  -o $@
