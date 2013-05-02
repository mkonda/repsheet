CFLAGS += -Wall -Werror -lhiredis
LOCAL_BUILD = build

.PHONY: clean setup

mod_repsheet:
	apxs -Wc $(CFLAGS) -c mod_repsheet.c

install:
	apxs -i -a mod_repsheet.la

mod_repsheet_local: setup
	$(LOCAL_BUILD)/bin/apxs -Wc $(CFLAGS) -c mod_repsheet.c

install_local: mod_repsheet_local
	$(LOCAL_BUILD)/bin/apxs -i -a mod_repsheet.la

setup:
	script/bootstrap

clean:
	rm -rf *.la *.lo *.slo *.o .libs *.dSYM backend/repsheet

clobber: clean
	rm -rf build vendor
