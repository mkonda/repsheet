require 'rack'
require 'logger'

class Example
  def call(env)
    [200, {"Content-Type" => "text/html"}, [env.inspect]]
  end
end

run Example.new
