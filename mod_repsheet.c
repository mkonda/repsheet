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
  int enabled;
  int timeout;
  int action;
  int port;
  int ttl;
  int length;
  const char *prefix;
  const char *host;
} repsheet_config;
static repsheet_config config;

const char *repsheet_set_enabled(cmd_parms *cmd, void *cfg, const char *arg)
{
  if (!strcasecmp(arg, "on")) config.enabled = 1; else config.enabled = 0;
  return NULL;
}

const char *repsheet_set_timeout(cmd_parms *cmd, void *cfg, const char *arg)
{
  config.timeout = atoi(arg) * 1000;
  return NULL;
}

const char *repsheet_set_host(cmd_parms *cmd, void *cfg, const char *arg)
{
  config.host = arg;
  return NULL;
}

const char *repsheet_set_port(cmd_parms *cmd, void *cfg, const char *arg)
{
  config.port = atoi(arg);
  return NULL;
}

const char *repsheet_set_ttl(cmd_parms *cmd, void *cfg, const char *arg)
{
  config.ttl = atoi(arg) * 60 * 60;
  return NULL;
}

const char *repsheet_set_length(cmd_parms *cmd, void *cfg, const char *arg)
{
  config.length = atoi(arg);
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
    AP_INIT_TAKE1("repsheetEnabled", repsheet_set_enabled, NULL, RSRC_CONF, "Enable or disable mod_repsheet"),
    AP_INIT_TAKE1("repsheetAction", repsheet_set_action, NULL, RSRC_CONF, "Set the action"),
    AP_INIT_TAKE1("repsheetPrefix", repsheet_set_prefix, NULL, RSRC_CONF, "Set the log prefix"),
    AP_INIT_TAKE1("repsheetRedisTimeout", repsheet_set_timeout, NULL, RSRC_CONF, "Set the Redis timeout"),
    AP_INIT_TAKE1("repsheetRedisHost", repsheet_set_host, NULL, RSRC_CONF, "Set the Redis host"),
    AP_INIT_TAKE1("repsheetRedisPort", repsheet_set_port, NULL, RSRC_CONF, "Set the Redis port"),
    AP_INIT_TAKE1("repsheetRedisTTL", repsheet_set_ttl, NULL, RSRC_CONF, "Set the Redis Expiry for keys (in hours)"),
    AP_INIT_TAKE1("repsheetRedisMaxLength", repsheet_set_length, NULL, RSRC_CONF, "Last n requests kept per IP"),
    { NULL }
  };

char *substr(char *string, int start, int end)
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

static int repsheet_recorder(request_rec *r)
{
  if (config.enabled == 1) {
    if (!ap_is_initial_req(r)) return DECLINED;
    apr_time_exp_t start;
    char           human_time[50];
    char           value[256]; // TODO: potential overflow here
    redisContext   *c;
    redisReply     *reply;

    struct timeval timeout = {0, (config.timeout > 0) ? config.timeout : 10000};

    c = redisConnectWithTimeout((char*) config.host, config.port, timeout);
    if (c == NULL || c->err) {
      if (c) {
        ap_log_error(APLOG_MARK, APLOG_ERR, 0, r->server, "%s Redis Connection Error: %s", config.prefix, c->errstr);
        redisFree(c);
      } else {
        ap_log_error(APLOG_MARK, APLOG_ERR, 0, r->server, "%s Connection Error: can't allocate redis context", config.prefix);
      }

      return DECLINED;
    }

    reply = redisCommand(c, "SISMEMBER repsheet %s", r->connection->remote_ip);
    if (reply->integer == 1) {
      if (config.action == BLOCK) {
        return HTTP_FORBIDDEN;
      } else {
        ap_log_error(APLOG_MARK, APLOG_ERR, 0, r->server, "%s IP Address %s was found on the repsheet. No action taken", config.prefix, r->connection->remote_ip);
        apr_table_set(r->headers_in, "X-Repsheet", "true");
      }
    }
    freeReplyObject(reply);

    apr_time_exp_gmt(&start, r->request_time);
    sprintf(human_time, "%d/%d/%d %d:%d:%d.%d", (start.tm_mon + 1), start.tm_mday, (1900 + start.tm_year), start.tm_hour, start.tm_min, start.tm_sec, start.tm_usec);
    sprintf(value, "%s,%s,%s,%s,%s", human_time, apr_table_get(r->headers_in, "User-Agent"), r->method, r->uri, r->args);

    freeReplyObject(redisCommand(c, "LPUSH %s %s", r->connection->remote_ip, value));
    freeReplyObject(redisCommand(c, "LTRIM 0 %d", (config.length - 1)));
    freeReplyObject(redisCommand(c, "EXPIRE %s %d", r->connection->remote_ip, config.ttl));
    redisFree(c);
  }
  return DECLINED;
}


static int repsheet_mod_security_filter(request_rec *r)
{
  if (config.enabled == 1) {
    redisContext   *c;
    redisReply     *reply;

    struct timeval timeout = {0, (config.timeout > 0) ? config.timeout : 10000};

    c = redisConnectWithTimeout((char*) config.host, config.port, timeout);
    if (c == NULL || c->err) {
      if (c) {
        ap_log_error(APLOG_MARK, APLOG_ERR, 0, r->server, "%s Redis Connection Error: %s", config.prefix, c->errstr);
        redisFree(c);
      } else {
        ap_log_error(APLOG_MARK, APLOG_ERR, 0, r->server, "%s Connection Error: can't allocate redis context", config.prefix);
      }

      return DECLINED;
    }

    char *waf_events = (char *)apr_table_get(r->headers_in, "X-WAF-Events");
    char *waf_score = (char *)apr_table_get(r->headers_in, "X-WAF-Score");

    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server, "%s X-WAF-Events %s", config.prefix, waf_events);
    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server, "%s X-WAF-Score %s", config.prefix, waf_score);

    if (waf_events && waf_score ) {
      int erroffset, i , rc, count = 0;
      int ovector[100];
      unsigned int offset = 0;
      unsigned int len = strlen(waf_events);
      const char *error;
      char *results[100];
      char *event, *prev_event;
      pcre *re;

      re = pcre_compile ("\\d{6}", PCRE_MULTILINE, &error, &erroffset, 0);

      while (offset < len && (rc = pcre_exec(re, 0, waf_events, len, offset, 0, ovector, sizeof(ovector))) >= 0) {
        for (i = 0; i < rc; i++) {
          event = substr(waf_events, ovector[2*i], ovector[2*i] + (ovector[2*i+1] - ovector[2*i]));
          if (count > 0 && event != prev_event) {
            freeReplyObject(redisCommand(c, "SADD %s %s", event, r->connection->remote_ip));
            freeReplyObject(redisCommand(c, "INCR %s:%s", event, r->connection->remote_ip));
            freeReplyObject(redisCommand(c, "SADD repsheet %s", r->connection->remote_ip));
            prev_event = event;
          }
        }
        count++;
        offset = ovector[1];
      }
    }

    redisFree(c);
  }

  return DECLINED;
}

static void register_hooks(apr_pool_t *pool)
{
  ap_hook_log_transaction(repsheet_recorder, NULL, NULL, APR_HOOK_LAST);
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

