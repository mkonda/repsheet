require 'spec_helper'

describe "Repsheet Bootstrap" do
  it "Redis is running" do
    Redis.new.ping.should == "PONG"
  end

  it "Apache is running" do
    Curl.get("http://127.0.0.1:8888").body_str.should == "<html><body><h1>It works!</h1></body></html>"
  end
end

describe "Repsheet Recorder" do
  it "Records the IP, User Agent, Method, URI, and Arguments during a request" do
    redis = Redis.new
    redis.flushdb

    Curl.get "http://127.0.0.1:8888"
    sleep(1)

    redis.llen("127.0.0.1").should == 1
  end

  it "Trims the number of entries for an IP to the number specified" do
    redis = Redis.new
    redis.flushdb

    c = Curl::Easy.new
    3.times do
      c.url = "http://127.0.0.1:8888"
      c.perform
    end

    redis.llen("127.0.0.1").should == 2
  end
end

describe "Repsheet ModSecurity Integration" do
  it "Creates the proper Redis keys when a security rule is triggered" do
    redis = Redis.new
    redis.flushdb

    Curl.get "http://127.0.0.1:8888?../../"

    redis.type("950103:127.0.0.1").should == "string"
    redis.type("950103").should == "set"
    redis.type("repsheet").should == "set"
  end

  it "Adds the offending IP address to the repsheet" do
    redis = Redis.new
    redis.flushdb

    redis.sismember("repsheet", "127.0.0.1").should be_false

    Curl.get "http://127.0.0.1:8888?../../"

    redis.sismember("repsheet", "127.0.0.1").should be_true
  end
end
