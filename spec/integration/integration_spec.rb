require 'spec_helper'

describe "Repsheet Bootstrap" do
  it "Redis is running" do
    redis = Redis.new
    redis.ping.should == "PONG"
  end

  it "Apache is running" do
    driver = Selenium::WebDriver.for :firefox
    driver.get "http://localhost:8888"
    driver.find_element(:xpath, '//h1').text.should == "It works!"
    driver.close
  end
end

describe "Repsheet Recorder" do
  it "Records the IP, User Agent, Method, URI, and Arguments during a request" do
    redis = Redis.new
    redis.flushdb
    driver = Selenium::WebDriver.for :firefox
    driver.get "http://localhost:8888"
    redis = Redis.new
    redis.llen("127.0.0.1").should == 1
    driver.close
  end

  it "Trims the number of entries for an IP to the number specified" do
    system("sed -i.bak 's/RepsheetRedisMaxLength 1000/RepsheetRedisMaxLength 2/' build/conf/httpd.conf")
    system("build/bin/apachectl restart")
    sleep(5)
    redis = Redis.new
    redis.flushdb
    driver = Selenium::WebDriver.for :firefox
    3.times do
      driver.get "http://localhost:8888"
    end
    redis = Redis.new
    redis.llen("127.0.0.1").should == 2
    system("sed -i.bak 's/RepsheetRedisMaxLength 2/RepsheetRedisMaxLength 1000/' build/conf/httpd.conf")
    driver.close
  end
end

describe "Repsheet ModSecurity Integration" do
  it "Creates the proper Redis keys when a security rule is triggered" do
    driver = Selenium::WebDriver.for :firefox
    driver.get "http://localhost:8888?../../"
    redis = Redis.new
    redis.type("950103:127.0.0.1").should == "string"
    redis.type("950103").should == "set"
    redis.type("repsheet").should == "set"
    driver.close
  end

  it "Adds the offending IP address to the repsheet" do
    redis = Redis.new
    redis.flushdb
    redis.sismember("repsheet", "127.0.0.1").should be_false
    driver = Selenium::WebDriver.for :firefox
    driver.get "http://localhost:8888?../../"
    redis.sismember("repsheet", "127.0.0.1").should be_true
    driver.close
  end
end
