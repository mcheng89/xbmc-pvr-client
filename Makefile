LIB=
INC=

default: pvr-tvserver.pvr

pvr-tvserver.pvr: client.cpp tvserver.cpp utils.cpp
	g++ -o pvr-tvserver.pvr -Wall -shared $(INC) client.cpp tvserver.cpp utils.cpp $(LIB)

clean:
	rm -f pvr-tvserver.pvr