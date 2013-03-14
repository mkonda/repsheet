# Repsheet

## Setup

This module requires [hiredis](https://github.com/redis/hiredis) to be installed. Attempts to embed and link it have been unsuccessful so far. As long as you have it installed there should be no issues.

If you are on OS X, you might have to run the following to make the compile tool-chain work properly with apxs:

```
ln -s /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/ /Applications/Xcode.app/Contents/Developer/Toolchains/OSX10.8.xctoolchain
```


## Apache Module

To build the module you can just run `make`. It will ensure that the module builds properly on your system. If you are running OS X, you might need to run `make mountain_lion_setup` to symlink the proper dev tools into place for apxs.

#### Installing

Just run `sudo make install`. You just need to enable the module using the `RepsheetEnabled` directive in your apache/httpd.conf file.

```
<IfModule repsheet_module>
  RepsheetEnabled On
  RepsheetRedisTimeout 5 # This is milliseconds
  RepsheetAction Notify|Block # Nofity adds a header to the downstream request. Block returns a 403
</IfModule>
```

## TODO

* Make the redis connection plumbing more robust
* Profile/Lint
