# Repsheet [![Build Status](https://secure.travis-ci.org/repsheet/repsheet.png)](http://travis-ci.org/repsheet/repsheet?branch=master)

> 
> **noun**  *Slang.*  
> A record kept by web site authorities of a machine's wrongdoings.
>
> **noun**  *Slang.*  
> A necessary tool in your web defense arsenal.
>
> **Origin:**  
> [2013]

Repsheet is a collection of tools to help improve awareness of robots and bad actors visiting your web applications. It is primarily an Apache web server module with a couple of addons that help organize and aggregate the data that it collects.

## What problem does it solve?

Repsheet attempts to solve the problem of automated defenses against robots and noisy attackers. It collects and records activity of the IP addresses that access your web sites and helps determine if they are misbehaving. If they are, they are put on the Repsheet. Once there, you can choose to outright deny them access to your site or warn your downstream applications that the requestor is a known bad actor and that the request should be handled differently. Essentially, it is a local reputation engine.

## How does it work?

Repsheet works by inspecting requests as they come into the web server and storing that activity in Redis. It does this in two ways. 

First, when a request comes in, the IP is checked to see if it has been flagged by Repsheet. If it has, it acts on that information based on the configuration set. If `RepsheetAction` is set to `Block`, Repsheet instructs Apache to return a 403. If `RepsheetAction` is set to `Notify`, then Repsheet will log to the Apache logs that the IP was found on the repsheet but no action was taken. Repsheet will then add the header `X-Repsheet: true` to the request to make downstream applications aware that the request is from a known bad source. Next, if the `RepsheetRecorder` directive is set to `On`, the the following details are recorded in Redis list using the key `<ip>:requests`

* Timestamp
* User Agent
* HTTP Method
* URI
* Arguments

These are recorded for every request the IP makes.

Second, if the `RepsheetFilter` directive is set to `On`, Repsheet looks at [ModSecurity](http://modsecurity.org) to see if the request triggered any rules. If so, [ModSecurity](http://modsecurity.org) places details into the request headers. Repsheet reads these headers and records information about which rules were triggered and stores them in Redis in the following fashion:

```
sadd <ruleid> <ip>:detected
incr <ip>:<rule>:count
set <ip>:repsheet true
```

In other words, each IP address that triggers a ModSecurity is put on the Repsheet and each rule that has been triggered is recorded as well as how many times that rule has been triggered. Finally, The IP address is flagged as being a repsheet offender.

The high level idea is captured in this drawing:

![Repsheet](https://raw.github.com/abedra/repsheet/master/doc/Repsheet.png)

## What are the benefits?

Repsheet provides some much needed intelligence and visualization around who is acting against your web site(s) and how they might be attacking. All of this data is available when the infrastructure is setup properly, but it isn't correlated or aggregated in any meaningful way. On top of these benefits, it allows for easy addition of an IP to the repsheet by just adding it manually in Redis. This means that you can plug in any activity processing systems you want and have them feed back into the Redis instance that Repsheet reads from. This makes the solution as flexible as you need it to be. It also leaves space for statistical analysis of traffic patterns to run in the background on this data and classify/detect anomalies that can be fed back into Repsheet.

## What are the downsides?

In short, more work. A Redis instance has to be spun up that can handle the data coming in to your site. If you get a lot of traffic, your Redis instance is going to grow. You can configure how many requests are stored for an IP address to limit a single IP address from blowing up your Redis database, but you still have to account for all the IPs that come into contact and how frequently they do so. 

Along with this, there is the potential for both [ModSecurity](http://modsecurity.org) and Repsheet to slow down your requests. The [ModSecurity](http://modsecurity.org) slow downs are mostly based on the rules you configure, but you should do some profiling to ensure that [ModSecurity](http://modsecurity.org) isn't causing to large of a delay. Since Repsheet contacts an external service during the request lifecycle in a blocking fashion, the request is slowed down for the duration of that external service call. Fortunately this is configurable in `RepsheetRedisTimeout`. You should keep this as low as possible, and the timeout value is always in milliseconds. This should be profiled as well to ensure that it doesn't cause unnecessary overhead.

## Setup

This module requires [hiredis](https://github.com/redis/hiredis), [apxs](http://httpd.apache.org/docs/2.2/programs/apxs.html), and [pcre](http://www.pcre.org/), and optionally [geoip](http://www.maxmind.com/en/geolocation_landing) (if you want GeoIP support) to be installed. If you want to run the integration tests, you will need to have [Ruby](http://www.ruby-lang.org/en/) and [RubyGems](http://rubygems.org/) installed. The integration tests use [Bundler](http://gembundler.com/), so you need to have that installed as well. The Ruby based programs have all been tested using Ruby 1.9.3.

If you are on OS X, you might have to run the following to make the compile tool-chain work properly with apxs:

```
ln -s /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/ /Applications/Xcode.app/Contents/Developer/Toolchains/OSX10.8.xctoolchain
```

#### Installation

Compilation is done via `apxs`. This is one of the simpler ways to deal with compilation/installation/activation of a module. Repsheet uses autotools to generate `configure` scripts and make files. If you want to build from source you will need to have the [check](http://check.sourceforge.net/) library installed as well as the dependencies listed above.

```
autogen.sh
./configure
make
sudo make install
```

To activate and configure repsheet you will need to set some directives. The following list explains what each directive is and what is does.

* `RepsheetEnabled <On|Off>` - Determines if Repsheet will do any processing
* `RepsheetRecorder <On|Off>` - Determines if request information will be stored
* `RepsheetFilter <On|Off>` - Determines if Repsheet will look for ModSecurity information
* `RepsheetGeoIP <On|Off>` - Determines if Repsheet will look at GeoIP information
* `RepsheetProxyHeaders <On|Off>` - Determines if Repsheet will look for the X-Forwarded-For header to determine remote ip
* `RepsheetAction <Notify|Block>` - Determines the action to take if an IP is found on the repsheet. `Notify` will send a header downstream and `Block` will return a `403`
* `RepsheetPrefix <prefix>` - Sets the logger prefix. This will precede any repsheet apache log lines
* `RepsheetRedisTimeout n` - Sets the time (in milliseconds) before the attempt to connect to redis will timeout and fail
* `RepsheetRedisHost <host>` - Sets the host for the Redis connection
* `RepsheetRedisTimeout <port>` - Sets the port for the Redis connection
* `RepsheetRedisMaxLength <max>` - Number of recorded requests a single IP can have before it is trimmed in Redis

Here's a complete example:

```
<IfModule repsheet_module>
  RepsheetEnabled On
  RepsheetRecorder On
  RepsheetFilter On
  RepsheetGeoIP On
  RepsheetProxyHeaders On
  RepsheetAction Notify
  RepsheetPrefix [repsheet]
  RepsheetRedisTimeout 5
  RepsheetRedisHost localhost
  RepsheetRedisPort 6379
  RepsheetRedisMaxLength 1000
</IfModule>
```

##### ModSecurity Settings

In order to take full advantage of Repsheet, you need to have the proper ModSecurity rules setup. This repository packages a working set of rules as an example. They are the  [OWASP Core Rules](https://github.com/SpiderLabs/owasp-modsecurity-crs) base rules plus the optional rule set that perfoms header tagging. You can and should tune these rules so that they work properly for your enviornment, but remember to have the header tagging rules in place or Repsheet will not be able to see what ModSecurity has alerted on.

## Running the Unit Tests

Repsheet has a set of C based unit tests for the core logic. You can run them with `make check` after you have setup your build environment as described above.


## Running the Integration Tests

Repsheet comes with a basic set of integration tests to ensure that things are working. In order to run these, run the following

```sh
script/bootstrap
autogen.sh
configure
make
build/bin/apxs -i -a src/mod_security.la
bundle install
rake
```

The `script/bootstrap` task will take some time. It downloads and compiles Apache and ModSecurity, and then compiles and installs Repsheet and configures everything to work together. Running `rake` launches some curl based tests that hit the site and exercise Repsheet, then test that everything was recorded properly in Redis.
