require 'sinatra'
require 'rack/recaptcha'

use Rack::Recaptcha, :public_key => 'KEY', :private_key => 'KEY'
helpers Rack::Recaptcha::Helpers

get '/' do
  @repsheet = request.env['X_REPSHEET'] ? true : false
  erb :index
end
