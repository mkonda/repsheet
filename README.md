# Repsheet

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

First, when a request comes in, the IP is checked to see if it is a member of the `repsheet` set. If it is, it acts on that information based on the configuration set. If `RepsheetAction` is set to `Block`, Repsheet instructs Apache to return a 403. If `RepsheetAction` is set to `Notify`, then Repsheet will log to the Apache logs that the IP was found on the repsheet but no action was taken. Repsheet will then add the header `X-Repsheet: true` to the request to make downstream applications aware that the request is from a known bad source. Next, the the following details are recorded in Redis under a list that keys off of the IP address:

* Timestamp
* User Agent
* HTTP Method
* URI
* Arguments

These are recorded for every request that IP makes. This key expires after a configurable amount of time, but the expiry is reset on every request. Essentially the key will fall off once the IP has been dormant for `RepsheetRedisTTL` hours.

Second, Repsheet looks at [ModSecurity](http://modsecurity.org) to see if the request triggered any rules. If that is the case, [ModSecurity](http://modsecurity.org) places details into the request headers. Repsheet reads these headers and records information about which rules were triggered and stores them in Redis in the following fashion:

```
sadd <ruleid> <ip>
incr <ruleid>:<ip>
sadd repsheet <ip>
```

In other words, each IP address that triggers a rule is stored in a set that keys off of the rule id. For each IP address that triggers a rule, an integer key that keys off the combined string of `ruleid:ip` is created and incremented any time the rule is triggered by the IP address. Finally, The IP address is added to the repsheet because it triggered one or more rules.

The high level idea is captured in this drawing:

![Repsheet](https://raw.github.com/abedra/repsheet/master/doc/Repsheet.png)

## What are the benefits?

Repsheet provides some much needed intelligence and visualization around who is acting against your web site(s) and how they might be attacking. All of this data is available when the infrastructure is setup properly, but it isn't correlated or aggregated in any meaningful way. On top of these benefits, it allows for easy addition of an IP to the repsheet by just adding it manually in Redis. This means that you can plug in any activity processing systems you want and have them feed back into the Redis instance that Repsheet reads from. This makes the solution as flexible as you need it to be. It also leaves space for statistical analysis of traffic patterns to run in the background on this data and classify/detect anomalies that can be fed back into Repsheet.

## What are the downsides?

In short, more work. A Redis instance has to be spun up that can handle the data coming in to your site. If you get a lot of traffic, your Redis instance is going to grow. You can configure the expiry of inactive IPs as well as how many entries each IP address can have at one time to limit a single IP address from blowing up your Redis database, but you still have to account for all the IPs that come into contact and how frequently they do so. 

Along with this, there is the potential for both [ModSecurity](http://modsecurity.org) and Repsheet to slow down your requests. The [ModSecurity](http://modsecurity.org) slow downs are mostly based on the rules you configure, but you should do some profiling to ensure that [ModSecurity](http://modsecurity.org) isn't causing to large of a delay. Since Repsheet contacts an external service during the request lifecycle in a blocking fashion, the request is slowed down for the duration of that external service call. Fortunately this is configurable in `RepsheetRedisTimeout`. You should keep this as low as possible, and the timeout value is always in milliseconds. This should be profiled as well to ensure that it doesn't cause unnecessary overhead.

## Setup

This module requires [hiredis](https://github.com/redis/hiredis) and [apxs](http://httpd.apache.org/docs/2.2/programs/apxs.html) to be installed. If you want to run the integration tests or the visualizer, you will need to have [Ruby](http://www.ruby-lang.org/en/) and [RubyGems](http://rubygems.org/) installed. Both the integration tests and the visualizer use [Bundler](http://gembundler.com/), so you need to have that installed as well. The Ruby based programs have all been tested using Ruby 1.9.3.

If you are on OS X, you might have to run the following to make the compile tool-chain work properly with apxs:

```
ln -s /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/ /Applications/Xcode.app/Contents/Developer/Toolchains/OSX10.8.xctoolchain
```

#### Installation

Compilation is done via `apxs`. This is one of the simpler ways to deal with compilation/installation/activation of a module. If you need to install manually, you can simply run `make`, copy the files to the right location, and add the configuration in your `httpd.conf`

```
make
sudo make install
```

To activate and configure repsheet you will need to set some directives. The following list explains what each directive is and what is does.

* `RepsheetEnabled <On|Off>` - Determines if the module will do any processing
* `RepsheetAction <Notify|Block>` - Determines the action to take if an IP is found on the repsheet. `Notify` will send a header downstream and `Block` will return a `403`
* `RepsheetPrefix <prefix>` - Sets the logger prefix. This will precede any repsheet apache log lines
* `RepsheetRedisTimeout n` - Sets the time (in milliseconds) before the attempt to connect to redis will timeout and fail
* `RepsheetRedisHost <host>` - Sets the host for the Redis connection
* `RepsheetRedisTimeout <port>` - Sets the port for the Redis connection
* `RepsheetRedisTTL <ttl>` - Number of hours before an IP entry will expire. If new activity from an IP is seen before expiry the expiry time will reset
* `RepsheetRedisMaxLength <ttl>` - Number of recorded requests a single IP can have before it is trimmed in Redis

Here's a complete example:

```
<IfModule repsheet_module>
  RepsheetEnabled On
  RepsheetAction Notify
  RepsheetPrefix [repsheet]
  RepsheetRedisTimeout 5
  RepsheetRedisHost localhost
  RepsheetRedisPort 6379
  RepsheetRedisTTL 24
  RepsheetRedisMaxLength 1000
</IfModule>
```

To ensure that stale IPs are cleaned up from the repsheet, you will need to run the Repsheet cleaner. It is recommended to run this on a schedule via cron. A binary named repsheet_cleaner is installed to `/usr/bin` during the install process. For example, if you want to run the cleaner every 5 minutes, you would create an entry in cron like so:

```
*/5 * * * * /usr/bin/repsheet_cleaner
```

## Running the Integration Tests

Repsheet comes with a basic set of integration tests to ensure that things are working. In order to run these, run the following

```sh
bundle install
make install_local
rake
```

The `make install_local` task will take some time. It downloads and compiles Apache and ModSecurity, and then compiles and installs Repsheet and configures everything to work together. Running `rake` launches some web-driver based tests that hit the site and trigger ModSecurity rules, then test that everything was recorded properly in Redis.
