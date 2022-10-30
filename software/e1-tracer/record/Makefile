CC=gcc
CFLAGS=`pkg-config libusb-1.0 --cflags` -O2 -Wall
LDLIBS=`pkg-config libusb-1.0 --libs`

OBJS=e1-tracer-record

all: $(OBJS)

e1-tracer-record: idt82v2081.o idt82v2081_usb.o e1-tracer-record.o

clean:
	rm -f $(OBJS) *.o
