LDFLAGS+=-losmocore -losmogsm
CFLAGS+=-Wall

all: e1_test e1_test_dieter

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^

e1_test: osmo_e1.o crc4itu.o e1_test.o
	$(CC) $(LDFLAGS) -o $@ $^

e1_test_dieter: osmo_e1.o crc4itu.o e1_test_dieter.o
	$(CC) $(LDFLAGS) -o $@ $^


clean:
	@rm *.o e1_test
