CC=gcc
CFLAGS=`pkg-config libusb-1.0 libosmocore libosmoabis --cflags` -O2 -Wall
LDLIBS=`pkg-config libusb-1.0 libosmocore --libs`

OBJS=main dump replay hdlc-decode-pipe

all: $(OBJS)

main: idt82v2081.o idt82v2081_usb.o main.o

dump: dump.o crc4itu.o osmo_e1f.o

clean:
	rm -f $(OBJS) *.o
