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
    driver = Selenium::WebDriver.for :firefox
    driver.get "http://localhost:8888"
    redis = Redis.new
    redis.lrange("127.0.0.1", 0, -1).count.should be > 0
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
