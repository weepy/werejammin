LIB := /usr/local/lib
INC = /usr/local/include

all:
	# gcc -O1 -o echo -I$(INC) -Wall echo.c $(LIB)/libuv.a 
	gcc -O1 -o capture -I$(INC) -Wall capture.c $(LIB)/libportaudio.a -framework CoreServices -framework CoreFoundation -framework AudioUnit -framework AudioToolbox -framework CoreAudio

PHONY: all
