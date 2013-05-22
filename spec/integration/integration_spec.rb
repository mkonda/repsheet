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

  describe "Actions" do
    it "Returns a 403 response if the actor is on the blacklist" do
      @redis.set("127.0.0.1:repsheet:blacklist", "true")
      Curl.get("http://127.0.0.1:8888").response_code.should == 403
    end

    it "Returns a 200 response if the actor is on the whitelist" do
      @redis.set("127.0.0.1:repsheet:blacklist", "true")
      @redis.set("127.0.0.1:repsheet:whitelist", "true")
      Curl.get("http://127.0.0.1:8888").response_code.should == 200
    end

    it "Returns a 200 response if the actor is on the repsheet but Notify is the default action" do
      @redis.set("127.0.0.1:repsheet", "true")
      Curl.get("http://127.0.0.1:8888").response_code.should == 200
    end
  end

  describe "Recorder" do
    it "Records the IP, User Agent, Method, URI, and Arguments during a request" do
      Curl.get "http://127.0.0.1:8888"

      @redis.llen("127.0.0.1:requests").should == 1
    end

    it "Records activity using the proper IP when behind a proxy" do
      http = Curl.get("http://127.0.0.1:8888") do |http|
        http.headers['X-Forwarded-For'] = '1.1.1.1'
      end

      @redis.llen("1.1.1.1:requests").should == 1
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

    it "Adds the offending IP address to the repsheet when behind a proxy" do
      @redis.get("1.1.1.1:repsheet").should be_false

      http = Curl.get("http://127.0.0.1:8888?../../") do |http|
        http.headers['X-Forwarded-For'] = '1.1.1.1'
      end

      @redis.get("1.1.1.1:repsheet").should be_true
    end
  end
end
