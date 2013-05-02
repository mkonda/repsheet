require 'curb'
require 'redis'

puts "Downloading AlienVault Reputation Database"
db = Curl.get("https://reputation.alienvault.com/reputation.snort").body_str.split("\n")

puts "Importing AlienVault data into Repsheet"
redis = Redis.new

db.each do |line|
  if line =~ /\b(?:[0-9]{1,3}\.){3}[0-9]{1,3}\b/
    redis.set("#{line.split('#').first.strip}:repsheet", "true")
  end
end
puts "Import complete"
