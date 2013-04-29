require 'sinatra'
require 'redis'
require 'json'

class Visualizer < Sinatra::Base
  get '/' do
    redis = Redis.new(:host => "localhost", :port => 6379)
    @data = {}
    offenders = redis.keys("*:repsheet").map {|o| o.split(":").first}
    offenders.each do |offender|
      @data[offender] = {"totals" => {}}
      redis.smembers("#{offender}:detected").each do |rule|
        @data[offender]["totals"][rule] = redis.get "#{offender}:#{rule}:count"
      end
    end
    @aggregate = Hash.new 0
    @data.each {|ip,data| data["totals"].each {|rule,count| @aggregate[rule] += count.to_i}}
    erb :index
  end
end

run Visualizer
