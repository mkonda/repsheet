CFLAGS += -Wall -Werror -lhiredis
LOCAL_BUILD = build

define CONFIG
LoadModule security2_module modules/mod_security2.so

<IfModule security2_module>
  Include conf/modsecurity/*.conf
</IfModule>

<IfModule repsheet_module>
  RepsheetEnabled On
  RepsheetAction Notify
  RepsheetPrefix [repsheet]
  RepsheetRedisTimeout 5
  RepsheetRedisHost localhost
  RepsheetRedisPort 6379
</IfModule>
endef

.PHONY: clean setup

mod_repsheet:
	apxs -Wc $(CFLAGS) -c mod_repsheet.c
install:
	apxs -i -a mod_repsheet.la
mod_repsheet_local: setup
	$(LOCAL_BUILD)/bin/apxs -Wc $(CFLAGS) -c mod_repsheet.c
export CONFIG
install_local: mod_repsheet_local
	$(LOCAL_BUILD)/bin/apxs -i -a mod_repsheet.la
	@echo "$$CONFIG" >> $(LOCAL_BUILD)/conf/httpd.conf
setup:
	script/bootstrap
clean:
	rm -rf *.la *.lo *.slo *.o .libs
clobber: clean
	rm -rf build vendor
