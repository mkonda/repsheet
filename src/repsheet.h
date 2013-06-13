#ifndef __REPSHEET_H
#define __REPSHEET_H

#include <stdlib.h>
#include <string.h>
#include "hiredis/hiredis.h"

#define ALLOW 0
#define NOTIFY 1
#define BLOCK 2

int repsheet_ip_lookup(redisContext *context, char *ip);
int repsheet_geoip_lookup(redisContext *context, const char *country);
void repsheet_record(redisContext *context, char *timestamp, const char *user_agent, const char *http_method, char* uri, char *arguments, char *ip, int max_length, int expiry);

#endif
