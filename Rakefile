require 'rake'
require 'rspec/core/rake_task'

RSpec::Core::RakeTask.new(:integration) do |t|
  t.pattern = "integration/**/*_spec.rb"
end

task :default => :integration
