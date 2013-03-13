# Repsheet

## Setup

This module requires [hiredis](https://github.com/redis/hiredis) to be installed. Attempts to embed and link it have been unsuccessful so far. As long as you have it installed there should be no issues.

## Apache Module

To build the module you can just run `make`. It will ensure that the module builds properly on your system. If you are running OS X, you might need to run `make mountain_lion_setup` to symlink the proper dev tools into place for apxs.

#### Installing

Just run `sudo make install`. You just need to enable the module using the `RepsheetEnabled` directive in your apache/httpd.conf file.

```
<IfModule repsheet_module>
  RepsheetEnabled On
  RepsheetRedisTimeout 5 # This is milliseconds (NOT YET SUPPORTED)
  RepsheetAction Notify # Adds a header to the downstream request. Block returns a 403 (NOT YET SUPPORTED)
</IfModule>
```

## TODO

* Add a directive to specify the redis connection timeout
* Make the redis connection plumbing more robust
* Profile/Lint
* Add support for looking up an ip to see if it has been deemed bad
* Add support for letting the user decide how to handle bad ips (Directive that takes either block or notify as arguments)
