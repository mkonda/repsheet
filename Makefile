# Had to do this on Mountain Lion to get apxs to compile things
# sudo ln -s /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/ /Applications/Xcode.app/Contents/Developer/Toolchains/OSX10.8.xctoolchain

mod_repsheet:
	apxs -c mod_repsheet.c
install:
	apxs -i -a -c mod_repsheet.c
clean:
	rm -rf *.la *.lo *.slo *.o .libs
