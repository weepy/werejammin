LIB = /usr/local/lib
INC = /usr/local/include

server:
	gcc -o bin/server -I$(INC) -Wall c/server.c $(LIB)/libuv.a

client:
	gcc -o bin/client -I$(INC) -Wall c/client.c $(LIB)/libuv.a

opus_with_port: 
	g++ -o bin/opus_with_port -I$(INC) -Wall c/opus_with_port.cpp $(LIB)/libportaudio.a $(LIB)/libopus.a -framework CoreServices -framework CoreFoundation -framework AudioUnit -framework AudioToolbox -framework CoreAudio

rec: 
	g++ -o bin/rec -I$(INC) -Wall c/rec.cpp $(LIB)/libportaudio.a $(LIB)/libopus.a -framework CoreServices -framework CoreFoundation -framework AudioUnit -framework AudioToolbox -framework CoreAudio


capturepp:
	g++ -std=c++11 -O1 -o bin/capture -I$(INC) -Wall c/capture.cpp $(LIB)/libportaudio.a -framework CoreServices -framework CoreFoundation -framework AudioUnit -framework AudioToolbox -framework CoreAudio


capture:
	gcc -O1 -o bin/capture -I$(INC) -Wall c/capture.c $(LIB)/libportaudio.a $(LIB)/libuv.a -framework CoreServices -framework CoreFoundation -framework AudioUnit -framework AudioToolbox -framework CoreAudio

jammin:
	g++ -std=c++11 -O1 -o bin/jammin -I$(INC) -Wall xcode/jammin/src/main.cpp $(LIB)/libuv.a $(LIB)/libportaudio.a $(LIB)/libopus.a -framework CoreServices -framework CoreFoundation -framework AudioUnit -framework AudioToolbox -framework CoreAudio

xcapture_clang:
	clang++ -std=c++11 -O1 -o bin/jammin -I$(INC) -Wall xcode/jammin/jammin/main.cpp $(LIB)/libuv.a $(LIB)/libportaudio.a -framework CoreServices -framework CoreFoundation -framework AudioUnit -framework AudioToolbox -framework CoreAudio

all:
	client capture server jammin

clean:
	rm bin/*

PHONY: all
