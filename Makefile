LIB := /usr/local/lib
INC = /usr/local/include

server:
	gcc -o bin/server -I$(INC) -Wall c/server.c $(LIB)/libuv.a

client:
	gcc -o bin/client -I$(INC) -Wall c/client.c $(LIB)/libuv.a


capture:
	g++ -std=c++11 -O1 -o bin/capture -I$(INC) -Wall c/capture.cpp $(LIB)/libportaudio.a -framework CoreServices -framework CoreFoundation -framework AudioUnit -framework AudioToolbox -framework CoreAudio 

all:
	client capture server

clean:
	rm bin/*

PHONY: all
