# Mod Repsheet Sample Application

## Configuring Apache

In order to have apache proxy down to the sample app, you will need to add the following configuration. I put this right after the repsheet directives.

```
<Proxy balancer://sample>
  BalancerMember http://127.0.0.1:9292
  Order deny,allow
  Allow from all
</Proxy>

RewriteEngine On
RewriteCond %{DOCUMENT_ROOT}/%{REQUEST_FILENAME} !-f
RewriteRule ^/(.*)$ balancer://sample%{REQUEST_URI} [P,QSA,L]
```

## Running the sample app

You will need to have Ruby/RubyGems installed. This app has been tested on Ruby 1.9.3. To setup and run do the following from the example folder:

``` sh
bundle install
rackup config.ru
```

Vising [http://localhost](http://localhost) should return the environment for the request. If you have the repsheet set in redis with your machines local ip (127.0.0.1 or ::1 usually), and your `RepsheetAction` directive set to `Notify`, you should see `"HTTP_X_REPSHEET"=>"true"` in the response.
