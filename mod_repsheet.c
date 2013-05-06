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

#include <stdio.h>
#include <pcre.h>
#include <assert.h>

#include "http_core.h"
#include "http_protocol.h"
#include "http_request.h"
#include "http_config.h"
#include "http_log.h"

#include "hiredis/hiredis.h"

#define UNDEFINED 0
#define NOTIFY 1
#define BLOCK 2

typedef struct {
  int repsheet_enabled;
  int recorder_enabled;
  int filter_enabled;
  int proxy_headers_enabled;
  int action;
  const char *prefix;

  const char *redis_host;
  int redis_port;
  int redis_timeout;
  int redis_ttl;
  int redis_max_length;
} repsheet_config;
static repsheet_config config;

const char *repsheet_set_enabled(cmd_parms *cmd, void *cfg, const char *arg)
{
  if (!strcasecmp(arg, "on")) {
    config.repsheet_enabled = 1;
  } else {
    config.repsheet_enabled = 0;
  }
  return NULL;
}

const char *repsheet_set_recorder_enabled(cmd_parms *cmd, void *cfg, const char *arg)
{
  if (!strcasecmp(arg, "on")) {
    config.recorder_enabled = 1;
  } else {
    config.recorder_enabled = 0;
  }
  return NULL;
}

const char *repsheet_set_filter_enabled(cmd_parms *cmd, void *cfg, const char *arg)
{
  if (!strcasecmp(arg, "on")) {
    config.filter_enabled = 1;
  } else {
    config.filter_enabled = 0;
  }
  return NULL;
}

const char *repsheet_set_proxy_headers_enabled(cmd_parms *cmd, void *cfg, const char *arg)
{
  if (!strcasecmp(arg, "on")) {
    config.proxy_headers_enabled = 1;
  } else {
    config.proxy_headers_enabled = 0;
  }
  return NULL;
}

const char *repsheet_set_timeout(cmd_parms *cmd, void *cfg, const char *arg)
{
  config.redis_timeout = atoi(arg) * 1000;
  return NULL;
}

const char *repsheet_set_host(cmd_parms *cmd, void *cfg, const char *arg)
{
  config.redis_host = arg;
  return NULL;
}

const char *repsheet_set_port(cmd_parms *cmd, void *cfg, const char *arg)
{
  config.redis_port = atoi(arg);
  return NULL;
}

const char *repsheet_set_ttl(cmd_parms *cmd, void *cfg, const char *arg)
{
  config.redis_ttl = atoi(arg) * 60 * 60;
  return NULL;
}

const char *repsheet_set_length(cmd_parms *cmd, void *cfg, const char *arg)
{
  config.redis_max_length = atoi(arg);
  return NULL;
}

const char *repsheet_set_prefix(cmd_parms *cmd, void *cfg, const char *arg)
{
  config.prefix = arg;
  return NULL;
}

const char *repsheet_set_action(cmd_parms *cmd, void *cfg, const char *arg)
{
  if (strcmp(arg, "Notify") == 0) {
    config.action = NOTIFY;
  } else if (strcmp(arg, "Block") == 0) {
    config.action = BLOCK;
  } else {
    config.action = UNDEFINED;
  }
  return NULL;
}

static const command_rec repsheet_directives[] =
  {
    AP_INIT_TAKE1("repsheetEnabled",         repsheet_set_enabled,               NULL, RSRC_CONF, "Enable or disable mod_repsheet"),
    AP_INIT_TAKE1("repsheetRecorder",        repsheet_set_recorder_enabled,      NULL, RSRC_CONF, "Enable or disable repsheet recorder"),
    AP_INIT_TAKE1("repsheetFilter",          repsheet_set_filter_enabled,        NULL, RSRC_CONF, "Enable or disable repsheet ModSecurity filter"),
    AP_INIT_TAKE1("repsheetProxyHeaders",    repsheet_set_proxy_headers_enabled, NULL, RSRC_CONF, "Enable or disable proxy header scanning"),
    AP_INIT_TAKE1("repsheetAction",          repsheet_set_action,                NULL, RSRC_CONF, "Set the action"),
    AP_INIT_TAKE1("repsheetPrefix",          repsheet_set_prefix,                NULL, RSRC_CONF, "Set the log prefix"),
    AP_INIT_TAKE1("repsheetRedisTimeout",    repsheet_set_timeout,               NULL, RSRC_CONF, "Set the Redis timeout"),
    AP_INIT_TAKE1("repsheetRedisHost",       repsheet_set_host,                  NULL, RSRC_CONF, "Set the Redis host"),
    AP_INIT_TAKE1("repsheetRedisPort",       repsheet_set_port,                  NULL, RSRC_CONF, "Set the Redis port"),
    AP_INIT_TAKE1("repsheetRedisTTL",        repsheet_set_ttl,                   NULL, RSRC_CONF, "Set the Redis Expiry for keys (in hours)"),
    AP_INIT_TAKE1("repsheetRedisMaxLength",  repsheet_set_length,                NULL, RSRC_CONF, "Last n requests kept per IP"),
    { NULL }
  };

static char *substr(char *string, int start, int end)
{
  char *ret = malloc(strlen(string) + 1);
  char *p = ret;
  char *q = &string[start];

  assert(ret != NULL);

  while(start < end) {
    *p++ = *q++;
    start++;
  }

  *p++ = '\0';

  return ret;
}

static redisContext *get_redis_context(request_rec *r)
{
  redisContext *context;
  struct timeval timeout = {0, (config.redis_timeout > 0) ? config.redis_timeout : 10000};

  context = redisConnectWithTimeout((char*) config.redis_host, config.redis_port, timeout);
  if (context == NULL || context->err) {
    if (context) {
      ap_log_error(APLOG_MARK, APLOG_ERR, 0, r->server, "%s Redis Connection Error: %s", config.prefix, context->errstr);
      redisFree(context);
    } else {
      ap_log_error(APLOG_MARK, APLOG_ERR, 0, r->server, "%s Connection Error: can't allocate redis context", config.prefix);
    }
    return NULL;
  } else {
    return context;
  }
}

static char *remote_address(request_rec *r)
{
  // TODO: Check that the value of the X-Forwarded-For header is
  // actually an IP address before returning it.
  if (config.proxy_headers_enabled) {
    char *address = (char*)apr_table_get(r->headers_in, "X-Forwarded-For");

    if (address == NULL) {
      return r->connection->remote_ip;
    }

    return address;
  } else {
    return r->connection->remote_ip;
  }
}

static int repsheet_offender(redisContext *context, request_rec *r)
{
  redisReply *reply;
  char *ip = remote_address(r);

  reply = redisCommand(context, "GET %s:repsheet:blacklist", ip);
  if (reply->str && strcmp(reply->str, "true") == 0) {
    freeReplyObject(reply);
    return BLOCK;
  }

  reply = redisCommand(context, "GET %s:repsheet", ip);
  if (reply->str && strcmp(reply->str, "true") == 0) {
    freeReplyObject(reply);
    return config.action;
  }

  freeReplyObject(reply);
  return 0;
}

static void record(redisContext *context, request_rec *r)
{
  apr_time_exp_t start;
  char           human_time[50];
  char           value[256]; // TODO: potential overflow here
  char           *ip = remote_address(r);

  apr_time_exp_gmt(&start, r->request_time);
  sprintf(human_time, "%d/%d/%d %d:%d:%d.%d", (start.tm_mon + 1), start.tm_mday, (1900 + start.tm_year), start.tm_hour, start.tm_min, start.tm_sec, start.tm_usec);
  sprintf(value, "%s,%s,%s,%s,%s", human_time, apr_table_get(r->headers_in, "User-Agent"), r->method, r->uri, r->args);

  freeReplyObject(redisCommand(context, "LPUSH %s:requests %s", ip, value));
  freeReplyObject(redisCommand(context, "LTRIM %s:requests 0 %d", ip, (config.redis_max_length - 1)));
}

static void process_waf_events(redisContext *context, request_rec *r, char *waf_events)
{
  int erroffset, i , rc, count = 0;
  int ovector[100];
  unsigned int offset = 0;
  unsigned int len = strlen(waf_events);
  char *event, *prev_event;
  const char *error;
  pcre *re;
  char *ip = remote_address(r);

  re = pcre_compile ("\\d{6}", PCRE_MULTILINE, &error, &erroffset, 0);

  while (offset < len && (rc = pcre_exec(re, 0, waf_events, len, offset, 0, ovector, sizeof(ovector))) >= 0) {
    for (i = 0; i < rc; i++) {
      event = substr(waf_events, ovector[2*i], ovector[2*i] + (ovector[2*i+1] - ovector[2*i]));
      if (count > 0 && event != prev_event) {
        freeReplyObject(redisCommand(context, "SADD %s:detected %s", ip, event));
        freeReplyObject(redisCommand(context, "INCR %s:%s:count", ip, event));
        freeReplyObject(redisCommand(context, "SET  %s:repsheet true", ip, 1));
        prev_event = event;
      }
    }
    count++;
    offset = ovector[1];
  }
}

static int repsheet_recorder(request_rec *r)
{
  if (!config.repsheet_enabled || !ap_is_initial_req(r)) {
    return DECLINED;
  }

  int offender;
  redisContext *context;

  context = get_redis_context(r);

  if (context == NULL) {
    return DECLINED;
  }

  char *ip = remote_address(r);

  offender = repsheet_offender(context, r);
  if (offender) {
    if (offender == BLOCK) {
      ap_log_error(APLOG_MARK, APLOG_ERR, 0, r->server, "%s %s was blocked by the repsheet", config.prefix, ip);
      return HTTP_FORBIDDEN;
    } else {
      ap_log_error(APLOG_MARK, APLOG_ERR, 0, r->server, "%s IP Address %s was found on the repsheet. No action taken", config.prefix, ip);
      apr_table_set(r->headers_in, "X-Repsheet", "true");
    }
  }

  if (config.recorder_enabled || !ap_is_initial_req(r)) {
    record(context, r);
  } else {
    return DECLINED;
  }

  redisFree(context);

  return DECLINED;
}

static int repsheet_mod_security_filter(request_rec *r)
{
  if (!config.repsheet_enabled || !config.filter_enabled) {
    return DECLINED;
  }

  char *waf_events = (char *)apr_table_get(r->headers_in, "X-WAF-Events");

  if (!waf_events) {
    return DECLINED;
  }

  redisContext *context;

  context = get_redis_context(r);

  if (context == NULL) {
    return DECLINED;
  }

  process_waf_events(context, r, waf_events);

  redisFree(context);

  return DECLINED;
}

static void register_hooks(apr_pool_t *pool)
{
  ap_hook_post_read_request(repsheet_recorder, NULL, NULL, APR_HOOK_LAST);
  ap_hook_fixups(repsheet_mod_security_filter, NULL, NULL, APR_HOOK_REALLY_LAST);
}

module AP_MODULE_DECLARE_DATA repsheet_module =
  {
    STANDARD20_MODULE_STUFF,
    NULL,                /* Per-directory configuration handler */
    NULL,                /* Merge handler for per-directory configurations */
    NULL,                /* Per-server configuration handler */
    NULL,                /* Merge handler for per-server configurations */
    repsheet_directives, /* Any directives we may have for httpd */
    register_hooks       /* Our hook registering function */
  };
