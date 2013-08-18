#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#include "hiredis/hiredis.h"

typedef struct {

} ngx_http_repsheet_loc_conf_t;

static redisContext *get_redis_context(ngx_http_request_t *r)
{
  redisContext *context;
  struct timeval timeout = {0, 5000};

  context = redisConnectWithTimeout("localhost", 6379, timeout);
  if (context == NULL || context->err) {
    if (context) {
      ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "%s Redis Connection Error: %s", "[repsheet]", context->errstr);
      redisFree(context);
    } else {
      ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "%s %s Connection Error: can't allocate redis context", "[repsheet]");
    }
    return NULL;
  } else {
    return context;
  }
}

static ngx_int_t ngx_http_repsheet_handler(ngx_http_request_t *r)
{
  if (r->main->internal) {
    return NGX_DECLINED;
  }

  redisContext *context;
  redisReply *reply;

  context = get_redis_context(r);
  if (context == NULL) {
    return NGX_DECLINED;
  }

  ngx_str_t address = r->connection->addr_text;

  reply = redisCommand(context, "GET %s:repsheet:blacklist", address.data);
  if (reply->str && strcmp(reply->str, "true") == 0) {
    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "%s %s was blocked by the repsheet", "[repsheet]", address.data);
    freeReplyObject(reply);
    redisFree(context);
    return NGX_HTTP_FORBIDDEN;
  }

  redisFree(context);

  r->main->internal = 1;
  return NGX_DECLINED;
}

static char *ngx_http_repsheet(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
  return NGX_CONF_OK;
}

static ngx_command_t ngx_http_repsheet_commands[] = {
  {
    ngx_string("enable_repsheet"),
    NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
    ngx_http_repsheet,
    0,
    0,
    NULL
  },

  ngx_null_command
};

static ngx_int_t ngx_http_repsheet_init(ngx_conf_t *cf)
{
  ngx_http_handler_pt *h;
  ngx_http_core_main_conf_t *cmcf;

  cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

  h = ngx_array_push(&cmcf->phases[NGX_HTTP_ACCESS_PHASE].handlers);
  if (h == NULL) {
    return NGX_ERROR;
  }

  *h = ngx_http_repsheet_handler;

  return NGX_OK;
}

static void *ngx_http_repsheet_create_loc_conf(ngx_conf_t *cf)
{
  ngx_http_repsheet_loc_conf_t *conf;

  conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_repsheet_loc_conf_t));
  if (conf == NULL) {
    return NULL;
  }

  return conf;
}

static char* ngx_http_repsheet_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
  ngx_http_repsheet_loc_conf_t *prev = parent;
  ngx_http_repsheet_loc_conf_t *conf = child;

  return NGX_CONF_OK;
}

static ngx_http_module_t ngx_http_repsheet_module_ctx = {
  NULL,                          /* preconfiguration */
  ngx_http_repsheet_init,        /* postconfiguration */

  NULL,                          /* create main configuration */
  NULL,                          /* init main configuration */

  NULL,                          /* create server configuration */
  NULL,                          /* merge server configuration */

  ngx_http_repsheet_create_loc_conf, /* create location configuration */
  ngx_http_repsheet_merge_loc_conf   /* merge location configuration */
};

ngx_module_t ngx_http_repsheet_module = {
  NGX_MODULE_V1,
  &ngx_http_repsheet_module_ctx,    /* module context */
  ngx_http_repsheet_commands,       /* module directives */
  NGX_HTTP_MODULE,               /* module type */
  NULL,                          /* init master */
  NULL,                          /* init module */
  NULL,                          /* init process */
  NULL,                          /* init thread */
  NULL,                          /* exit thread */
  NULL,                          /* exit process */
  NULL,                          /* exit master */
  NGX_MODULE_V1_PADDING
};
