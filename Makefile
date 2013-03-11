mod_repsheet:
	apxs -c mod_repsheet.c
install:
	apxs -i -a -c mod_repsheet.c
mountain_lion_setup:
	ln -s /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/ /Applications/Xcode.app/Contents/Developer/Toolchains/OSX10.8.xctoolchain
clean:
	rm -rf *.la *.lo *.slo *.o .libs
