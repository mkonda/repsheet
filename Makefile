HIREDIS_DIR=hiredis
HIREDIS_HEADERS = $(HIREDIS_DIR)/*.h
CFLAGS += -I$(HIREDIS_DIR)
CXXFLAGS += -g -Wall -Wextra

mod_repsheet:
	apxs -c mod_repsheet.c
install:
	apxs -i -a mod_repsheet.la
mountain_lion_setup:
	ln -s /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/ /Applications/Xcode.app/Contents/Developer/Toolchains/OSX10.8.xctoolchain
clean:
	rm -rf *.la *.lo *.slo *.o .libs hiredis_test test
clobber: clean
	rm -rf hiredis
hiredis_test.o: hiredis_test.c
	script/bootstrap
	gcc $(CFLAGS) $(CXXFLAGS) -c hiredis_test.c
hiredis_test: hiredis_test.o $(HIREDIS_DIR)/libhiredis.a
	gcc $(CFLAGS) $(CXXFLAGS) $^ -o $@
