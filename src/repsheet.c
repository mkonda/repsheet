#include "hiredis/hiredis.h"
#include "repsheet.h"

int repsheet_ip_lookup(redisContext *context, char *ip)
{
  redisReply *reply;

  reply = redisCommand(context, "GET %s:repsheet:whitelist", ip);
  if (reply->str && strcmp(reply->str, "true") == 0) {
    freeReplyObject(reply);
    return ALLOW;
  }

  reply = redisCommand(context, "GET %s:repsheet:blacklist", ip);
  if (reply->str && strcmp(reply->str, "true") == 0) {
    freeReplyObject(reply);
    return BLOCK;
  }

  reply = redisCommand(context, "GET %s:repsheet", ip);
  if (reply->str && strcmp(reply->str, "true") == 0) {
    freeReplyObject(reply);
    return NOTIFY;
  }

  freeReplyObject(reply);
  return ALLOW;
}

int repsheet_geoip_lookup(redisContext *context, const char *country)
{
  if (country == NULL) {
    return ALLOW;
  }

  redisReply *reply;

  reply = redisCommand(context, "SISMEMBER repsheet:countries %s", country);
  if (reply && reply->integer == 1) {
    freeReplyObject(reply);
    return NOTIFY;
  }

  freeReplyObject(reply);
  return ALLOW;
}

void repsheet_record(redisContext *context, char *timestamp, const char *user_agent, const char *http_method, char *uri, char *arguments, char *ip, int max_length, int expiry)
{
  char *t, *ua, *method, *u, *args, *rec;

  if (timestamp == NULL) {
    t = malloc(2);
    sprintf(t, "%s", "-");
  } else {
    t = malloc(strlen(timestamp) + 1);
    sprintf(t, "%s", timestamp);
  }

  if (user_agent == NULL) {
    ua = malloc(2);
    sprintf(ua, "%s", "-");
  } else {
    ua = malloc(strlen(user_agent) + 1);
    sprintf(ua, "%s", user_agent);
  }

  if (http_method == NULL) {
    method = malloc(2);
    sprintf(method, "%s", "-");
  } else {
    method = malloc(strlen(http_method) + 1);
    sprintf(method, "%s", http_method);
  }

  if (uri == NULL) {
    u = malloc(2);
    sprintf(u, "%s", "-");
  } else {
    u = malloc(strlen(uri) + 1);
    sprintf(u, "%s", uri);
  }

  if (arguments == NULL) {
    args = malloc(2);
    sprintf(args, "%s", "-");
  } else {
    args = malloc(strlen(arguments) + 1);
    sprintf(args, "%s", arguments);
  }

  rec = (char*)malloc(strlen(t) + strlen(ua) + strlen(method) + strlen(u) + strlen(args) + 9);
  sprintf(rec, "%s, %s, %s, %s, %s", t, ua, method, u, args);

  freeReplyObject(redisCommand(context, "LPUSH %s:requests %s", ip, rec));
  freeReplyObject(redisCommand(context, "LTRIM %s:requests 0 %d", ip, (max_length - 1)));
  if (expiry > 0) {
    freeReplyObject(redisCommand(context, "EXPIRE %s:requests %d", ip, expiry));
  }
}
