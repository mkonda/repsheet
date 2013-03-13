#include <stdio.h>
#include "http_core.h"
#include "http_config.h"
#include "http_log.h"
#include "hiredis/hiredis.h"

typedef struct { int enabled; } repsheet_config;
static repsheet_config config;

const char *repsheet_set_enabled(cmd_parms *cmd, void *cfg, const char *arg)
{
  if (!strcasecmp(arg, "on")) config.enabled = 1; else config.enabled = 0;
  return NULL;
}

static const command_rec repsheet_directives[] =
  {
    AP_INIT_TAKE1("repsheetEnabled", repsheet_set_enabled, NULL, RSRC_CONF, "Enable or disable mod_repsheet"),
    { NULL }
  };

static int repsheet_handler(request_rec *r)
{
  if (config.enabled == 1) {
    if (strcmp(r->uri, "/favicon.ico")) {

      apr_time_exp_t start;
      char human_time[50];
      char value[256];
      redisContext *c;
      redisReply   *reply;
      struct timeval timeout = {0, 5000};

      apr_time_exp_gmt(&start, r->request_time);
      sprintf(human_time, "%d/%d/%d %d:%d:%d.%d", (start.tm_mon + 1), start.tm_mday, (1900 + start.tm_year), start.tm_hour, start.tm_min, start.tm_sec, start.tm_usec);
      sprintf(value, "%s,%s,%s,%s,%s", human_time, apr_table_get(r->headers_in, "User-Agent"), r->method, r->uri, r->args);

      c = redisConnectWithTimeout((char*) "127.0.0.1", 6379, timeout);
      if (c == NULL || c->err) {
        if (c) {
          ap_log_error(APLOG_MARK, APLOG_ERR, 0, r->server, "Redis Connection Error: %s\n", c->errstr);
          redisFree(c);
        } else {
          ap_log_error(APLOG_MARK, APLOG_ERR, 0, r->server, "Connection Error: can't allocate redis context\n");
        }
        return DECLINED;
      }

      reply = redisCommand(c, "RPUSH %s %s", r->connection->remote_ip, value);
      ap_log_error(APLOG_MARK, APLOG_ERR, 0, r->server, "[repsheet] Added data for %s with result %s", r->connection->remote_ip, reply->str);
      freeReplyObject(reply);
      redisFree(c);
    }
  }

  ap_log_error(APLOG_MARK, APLOG_ERR, 0, r->server, "[repsheet] PONG");
  return DECLINED;
}

static void register_hooks(apr_pool_t *pool)
{
  ap_hook_post_read_request(repsheet_handler, NULL, NULL, APR_HOOK_FIRST);
}

module AP_MODULE_DECLARE_DATA repsheet_module =
  {
    STANDARD20_MODULE_STUFF,
    NULL,               /* Per-directory configuration handler */
    NULL,               /* Merge handler for per-directory configurations */
    NULL,               /* Per-server configuration handler */
    NULL,               /* Merge handler for per-server configurations */
    repsheet_directives, /* Any directives we may have for httpd */
    register_hooks      /* Our hook registering function */
  };
