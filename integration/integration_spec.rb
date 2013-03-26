require 'rspec'
require 'redis'
require 'capybara'

describe "Repsheet" do
  it "has the proper components running" do
    redis = Redis.new
    redis.ping.should == "PONG"
  end
end
