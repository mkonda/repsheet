require 'sinatra'
require 'redis'
require 'json'

class Visualizer < Sinatra::Base
  get '/' do
    @data = {:rules => {}, :ips => {}}
    redis = Redis.new(:host => "localhost", :port => 6379)
    rules = redis.keys("9?????")
    rules.each do |rule|
      count = 0
      offenders = redis.smembers(rule)
      offenders.each do |offender|
        @data[:ips][offender] ||= {}
        @data[:ips][offender][rule] = 0
        current = redis.get("#{rule}:#{offender}").to_i
        @data[:ips][offender][rule] += current
        count += current
      end
      @data[:rules][rule] = count
    end
    erb :index
  end
end

run Visualizer
