require 'redis'

r = Redis.new

counts = Hash.new(0)
r.keys("*:*:count").each do |data|
  offender = data.split(":").first
  blacklist = r.get("#{offender}:repsheet:blacklist") == "true" ? true : false
  next if blacklist
  score = r.get(data).to_i
  counts[offender] += score
end

if counts.size > 0
  top = counts.sort_by {|k,v| -v}.take(10)
  puts "Top Offenders Not Already Blacklisted"
  top.each do |offender, count|
    puts "  #{offender}: #{count}"
  end
else
  puts "There are no new offenders to report"
end
