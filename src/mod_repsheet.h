/*
  Copyright 2013 Aaron Bedra

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#define REPSHEET_VERSION "0.9"
#define REPSHEET_URL "http://getrepsheet.com"

typedef struct {
  int repsheet_enabled;
  int recorder_enabled;
  int filter_enabled;
  int proxy_headers_enabled;
  int geoip_enabled;
  int action;
  const char *prefix;

  const char *redis_host;
  int redis_port;
  int redis_timeout;
  int redis_max_length;
  int redis_expiry;
} repsheet_config;
static repsheet_config config;

