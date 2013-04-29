require 'spec_helper'

describe "Integration Specs" do

  before do
    @redis = Redis.new
    @redis.flushdb
  end

  after  { @redis.flushdb }

  describe "Bootstrap" do
    it "Redis is running" do
      @redis.ping.should == "PONG"
    end

    it "Apache is running" do
      Curl.get("http://127.0.0.1:8888").body_str.should == "<html><body><h1>It works!</h1></body></html>"
    end
  end

  describe "Recorder" do
    it "Records the IP, User Agent, Method, URI, and Arguments during a request" do
      Curl.get "http://127.0.0.1:8888"

      @redis.llen("127.0.0.1:requests").should == 1
    end
  end

  describe "ModSecurity Integration" do
    it "Creates the proper Redis keys when a security rule is triggered" do
      Curl.get "http://127.0.0.1:8888?../../"

      @redis.type("127.0.0.1:requests").should == "list"
      @redis.type("127.0.0.1:detected").should == "set"
      @redis.type("127.0.0.1:repsheet").should == "string"
      @redis.type("127.0.0.1:950103:count").should == "string"
    end

    it "Adds the offending IP address to the repsheet" do
      @redis.get("127.0.0.1:repsheet").should be_false

      Curl.get "http://127.0.0.1:8888?../../"

      @redis.get("127.0.0.1:repsheet").should be_true
    end
  end
end
