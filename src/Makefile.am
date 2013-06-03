AUTOMAKE_OPTIONS = -Wno-override
lib_LTLIBRARIES = mod_repsheet.la
mod_repsheet_la_SOURCES = proxy.c mod_security.c repsheet.c mod_repsheet.c
mod_repsheet_la_LDFLAGS = -module -lhiredis
AM_CFLAGS = `apr-1-config --cflags`
AM_CPPFLAGS = -I@APACHE_INCLUDE_DIR@ `apr-1-config --cppflags --includes`

install:
	cp @HTTPD_CONF@ /tmp/httpd.conf.bak
	@APXS@ -i -a mod_repsheet.la
