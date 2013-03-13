HIREDIS_DIR=hiredis
HIREDIS_HEADERS = $(HIREDIS_DIR)/*.h
CFLAGS += -lhiredis
CXXFLAGS += -g -Wall -Wextra

mod_repsheet:
	apxs -Wc $(CFLAGS) -c mod_repsheet.c
install:
	apxs -i -a mod_repsheet.la
mountain_lion_setup:
	ln -s /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/ /Applications/Xcode.app/Contents/Developer/Toolchains/OSX10.8.xctoolchain
clean:
	rm -rf *.la *.lo *.slo *.o .libs hiredis_test test
clobber: clean
	rm -rf hiredis
