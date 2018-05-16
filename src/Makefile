LDFLAGS+=`pkg-config --libs libosmocore libosmogsm`
CFLAGS+=-Wall `pkg-config --cflags libosmocore libosmogsm`

all: e1_test e1_test_dieter

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^

e1_test: osmo_e1f.o crc4itu.o e1_test.o
	$(CC) $(LDFLAGS) -o $@ $^

e1_test_dieter: osmo_e1f.o crc4itu.o e1_test_dieter.o
	$(CC) $(LDFLAGS) -o $@ $^


clean:
	@rm -f *.o e1_test e1_test_dieter
