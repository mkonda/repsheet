CFLAGS += -Wall -Werror -lhiredis

mod_repsheet:
	apxs -Wc $(CFLAGS) -c mod_repsheet.c
install:
	apxs -i -a mod_repsheet.la
clean:
	rm -rf *.la *.lo *.slo *.o .libs
