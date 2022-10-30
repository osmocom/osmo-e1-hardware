CC=gcc
CFLAGS=`pkg-config libusb-1.0 libosmocore libosmoabis --cflags` -O2 -Wall
LDLIBS=`pkg-config libusb-1.0 libosmocore --libs`

OBJS=dump replay hdlc-decode-pipe

all: $(OBJS)

dump: dump.o crc4itu.o osmo_e1f.o

clean:
	rm -f $(OBJS) *.o
