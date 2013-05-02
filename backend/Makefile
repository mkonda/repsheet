CFLAGS += -Wall -Werror

.PHONY: clean

repsheet.o:
	gcc $(CFLAGS) -c repsheet.c

backend: repsheet.o
	gcc repsheet.o -o repsheet -lhiredis

clean:
	rm -rf *.la *.lo *.slo *.o .libs *.dSYM repsheet
