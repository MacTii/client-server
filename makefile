CFLAGS=-ggdb -Wall -pedantic -pthread

%: %.c
	gcc $(CFLAGS) $< -o $@ -lm

EXECS = client server

all: $(EXECS)

client: client.c

server: server.c

.PHONY: clean all

clean:
	rm -f $(EXECS)