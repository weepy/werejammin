LIB = /usr/local/lib
INC = /usr/local/include

server:
	gcc -o bin/server -I$(INC) -Wall c/server.c $(LIB)/libuv.a

client:
	gcc -o bin/client -I$(INC) -Wall c/client.c $(LIB)/libuv.a


capturepp:
	g++ -std=c++11 -O1 -o bin/capture -I$(INC) -Wall c/capture.cpp $(LIB)/libportaudio.a -framework CoreServices -framework CoreFoundation -framework AudioUnit -framework AudioToolbox -framework CoreAudio


capture:
	gcc -O1 -o bin/capture -I$(INC) -Wall c/capture.c $(LIB)/libportaudio.a $(LIB)/libuv.a -framework CoreServices -framework CoreFoundation -framework AudioUnit -framework AudioToolbox -framework CoreAudio

jammin:
	g++ -std=c++11 -O1 -o bin/jammin -I$(INC) -Wall xcode/jammin/jammin/main.cpp $(LIB)/libuv.a $(LIB)/libportaudio.a -framework CoreServices -framework CoreFoundation -framework AudioUnit -framework AudioToolbox -framework CoreAudio

xcapture_clang:
	clang++ -std=c++11 -O1 -o bin/jammin -I$(INC) -Wall xcode/jammin/jammin/main.cpp $(LIB)/libuv.a $(LIB)/libportaudio.a -framework CoreServices -framework CoreFoundation -framework AudioUnit -framework AudioToolbox -framework CoreAudio

all:
	client capture server jammin

clean:
	rm bin/*

PHONY: all
