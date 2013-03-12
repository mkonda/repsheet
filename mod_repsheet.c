#include <stdio.h>
#include "http_core.h"
#include "http_config.h"
#include "http_log.h"

typedef struct {
  int enabled;
} repsheet_config;

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
  apr_time_exp_t start;
  char human_time[50];

  apr_time_exp_gmt(&start, r->request_time);
  sprintf(human_time, "%d/%d/%d %d:%d:%d.%d", (start.tm_mon + 1), start.tm_mday, (1900 + start.tm_year), start.tm_hour, start.tm_min, start.tm_sec, start.tm_usec);

  if (config.enabled == 1) {
    if (strcmp(r->uri, "/favicon.ico")) {
      ap_log_error(APLOG_MARK, APLOG_ERR, 0, r->server, "[repsheet] - %s, %s, %s, %s, %s, %s", human_time, r->connection->remote_ip, apr_table_get(r->headers_in, "User-Agent"), r->method, r->uri, r->args);
    }
  }
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
