LDFLAGS+=-losmocore -losmogsm
CFLAGS+=-Wall

all: e1_test

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^

e1_test: osmo_e1.o crc4itu.o e1_test.o
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	@rm *.o e1_test
