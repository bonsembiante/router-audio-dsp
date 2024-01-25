OBJS = main.o ring_buffer.o sound.o processor.o
OUTPUT = delay_processor

ifeq ($(strip $(PROFILE)),router)

# For router building run:
# $ PROFILE=router make

PATH=/opt/trendchip/mips-linux-uclibc/usr/bin:$(PATH)
HOST=mips-linux
CROSS=mips-linux-uclibc-
CROSS_COMPILE=$(CROSS)
CC=mips-linux-uclibc-gcc
CPP=mips-linux-uclibc-g++
STRIP=mips-linux-uclibc-strip
SYSROOT=/opt/trendchip/mips-linux-uclibc
CFG_CFLAGS = -mips32r2 -muclibc
OBJ_CFLAGS = -g -c -Wall -D PCM_FORMAT_BE
CPPFLAGS = -I/opt/router/alsa-lib/include
LDFLAGS = -L/opt/router/alsa-lib/lib
LDLIBS = -lasound

else

# For PC building run:
# $ PROFILE=pc make
# or just
# $ make

CC=gcc
CPP=g++
CFG_CFLAGS = -pthread
OBJ_CFLAGS = -g -c -Wall
CPPFLAGS = 
LDFLAGS = 
LDLIBS = -lasound

endif

all: $(OBJS)
	$(CPP) $(CFG_CFLAGS) $(LDFLAGS) -o build/$(OUTPUT) $(OBJS) $(CPPFLAGS) $(LDLIBS)

main.o: main.cpp
	$(CPP) $(OBJ_CFLAGS) $(LDFLAGS) main.cpp $(CPPFLAGS) $(LDLIBS)

ring_buffer.o: ring_buffer.cpp
	$(CPP) $(OBJ_CFLAGS) $(LDFLAGS) ring_buffer.cpp $(CPPFLAGS) $(LDLIBS)

sound.o: sound.cpp sound.h
	$(CPP) $(OBJ_CFLAGS) $(LDFLAGS) sound.cpp $(CPPFLAGS) $(LDLIBS)

processor.o: processor.cpp processor.h
	$(CPP) $(OBJ_CFLAGS) $(LDFLAGS) processor.cpp $(CPPFLAGS) $(LDLIBS)

clean:
	rm $(OBJS) build/$(OUTPUT)