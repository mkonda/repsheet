# Repsheet

## Setup

This module requires [hiredis](https://github.com/redis/hiredis) to be installed. Attempts to embed and link it have been unsuccessful so far. As long as you have it installed there should be no issues.

If you are on OS X, you might have to run the following to make the compile tool-chain work properly with apxs:

```
ln -s /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/ /Applications/Xcode.app/Contents/Developer/Toolchains/OSX10.8.xctoolchain
```


## Apache Module

#### Installing

Compilation is done via `apxs`. This is one of the simpler ways to deal with compilation/installation/activation of a module. If you need to install manually, you can simply run `make`, copy the files to the right location, and add the configuration in your `httpd.conf`

```
make
sudo make install
```

#### Setup

To activate and configure repsheet you will need to set some directives. The following list explains what each directive is and what is does.

* `RepsheetEnabled <On|Off>` - Determines if the module will do any processing
* `RepsheetAction <Notify|Block>` - Determines the action to take if an IP is found on the repsheet. `Notify` will send a header downstream and `Block` will return a `403`
* `RepsheetPrefix <prefix>` - Sets the logger prefix. This will precede any repsheet apache log lines
* `RepsheetRedisTimeout n` - Sets the time (in milliseconds) before the attempt to connect to redis will timeout and fail
* `RepsheetRedisHost <host>` - Sets the host for the Redis connection
* `RepsheetRedisTimeout <port>` - Sets the port for the Redis connection

Here's a complete example:

```
<IfModule repsheet_module>
  RepsheetEnabled On
  RepsheetAction Notify
  RepsheetPrefix [repsheet]
  RepsheetRedisTimeout 5
  RepsheetRedisHost localhost
  RepsheetRedisPort 6379
</IfModule>
```

## TODO

* Add directives for redis host, redis port, and redis password
* Add simple backend app to ensure proper data gets sent down via proxy
* Setup end to end tests with a web driver that confirm all the functionality
* Make the redis connection plumbing more robust
* Profile/Lint
