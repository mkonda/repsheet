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

  describe "Proxy Filtering" do
    it "Properly determines the IP address when multiple proxies are present in X-Forwarded-For" do
      http = Curl.get("http://127.0.0.1:8888?../../") do |http|
        http.headers['X-Forwarded-For'] = '8.8.8.8, 12.34.56.78, 98.76.54.32'
      end
      @redis.lrange("8.8.8.8:requests", 0, -1).size.should == 1
    end

    it "Ignores user submitted noise in X-Forwarded-For" do
      http = Curl.get("http://127.0.0.1:8888?../../") do |http|
        http.headers['X-Forwarded-For'] = '\x5000 8.8.8.8, 12.34.56.78, 98.76.54.32'
      end
      @redis.lrange("8.8.8.8:requests", 0, -1).size.should == 1
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

    it "Properly sets the expiry" do
      http = Curl.get("http://127.0.0.1:8888?../../") do |http|
        http.headers['X-Forwarded-For'] = '1.1.1.1'
      end

      @redis.ttl("1.1.1.1:requests").should > 1
      @redis.ttl("1.1.1.1:detected").should > 1
    end
  end

  describe "ModSecurity Integration" do
    it "Creates the proper Redis keys when a security rule is triggered" do
      Curl.get "http://127.0.0.1:8888?../../"

      @redis.type("127.0.0.1:detected").should == "zset"
      @redis.type("127.0.0.1:repsheet").should == "string"
    end

    it "Adds the offending IP address to the repsheet" do
      @redis.get("127.0.0.1:repsheet").should be_false

      Curl.get "http://127.0.0.1:8888?../../"

      @redis.get("127.0.0.1:repsheet").should be_true
    end

    it "Properly sets and increments the waf events in <ip>:detected" do
      Curl.get "http://127.0.0.1:8888?../../"

      @redis.zscore("127.0.0.1:detected", "950103").should == 1.0
      @redis.zscore("127.0.0.1:detected", "960009").should == 1.0
      @redis.zscore("127.0.0.1:detected", "960017").should == 1.0

      Curl.get "http://127.0.0.1:8888?../../"

      @redis.zscore("127.0.0.1:detected", "950103").should == 2.0
      @redis.zscore("127.0.0.1:detected", "960009").should == 2.0
      @redis.zscore("127.0.0.1:detected", "960017").should == 2.0
    end

    it "Adds the offending IP address to the repsheet when behind a proxy" do
      @redis.get("1.1.1.1:repsheet").should be_false

      http = Curl.get("http://127.0.0.1:8888?../../") do |http|
        http.headers['X-Forwarded-For'] = '1.1.1.1'
      end

      @redis.get("1.1.1.1:repsheet").should be_true
    end
  end

  describe "GeoIP Blacklisting" do
    it "Adds the actor to the repsheet if the actor's origin country is on the suspicious countries list" do
      @redis.sadd("repsheet:countries", "US")
      http = Curl.get("http://127.0.0.1:8888?../../") do |http|
        http.headers['X-Forwarded-For'] = '8.8.8.8'
      end
      @redis.get("8.8.8.8:repsheet").should be_true
    end
  end
end
