require 'rake'
require 'rspec/core/rake_task'
require 'redis'

RSpec::Core::RakeTask.new(:integration) do |t|
  t.pattern = "spec/**/*_spec.rb"
end

namespace :services do
  task :start do
    print "Starting Services: "
    Rake::Task["apache:start"].invoke
    Rake::Task["redis:start"].invoke
  end

  task :stop do
    print "Shutting Down Services: "
    Rake::Task["apache:stop"].invoke
    Rake::Task["redis:stop"].invoke
  end
end

namespace :apache do
  task :start do
    print "Apache, "
    `build/bin/apachectl restart`
    sleep 1
  end

  task :stop do
    print "Apache, "
    `build/bin/apachectl stop`
  end
end

namespace :redis do
  task :start do
    print "Redis\n"
    `redis-server etc/redis.conf`
  end

  task :stop do
    print "Redis\n"
    redis = Redis.new
    redis.shutdown
  end
end


task :default => ["services:start", :integration, "services:stop"]
