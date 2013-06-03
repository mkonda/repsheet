redisContext *get_redis_context(request_rec *r)
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
