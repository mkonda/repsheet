HIREDIS_DIR=hiredis
HIREDIS_HEADERS = $(HIREDIS_DIR)/*.h
CFLAGS += -lhiredis

mod_repsheet:
	apxs -Wc $(CFLAGS) -c mod_repsheet.c
install:
	apxs -i -a mod_repsheet.la
clean:
	rm -rf *.la *.lo *.slo *.o .libs hiredis_test test
clobber: clean
	rm -rf hiredis
