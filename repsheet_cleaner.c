#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "hiredis/hiredis.h"

int main(int argc, char *argv[])
{
  int i, c;
  redisContext *context;
  redisReply *repsheet_members, *reply;
  char *host = "localhost";
  int port = 6379;

  while((c = getopt (argc, argv, "h:p:")) != -1)
    switch(c)
      {
      case 'h':
        host = optarg;
        break;
      case 'p':
        port = atoi(optarg);
        break;
      case '?':
        return 1;
      default:
        abort();
      }

  printf("Starting Repsheet cleanup on %s:%d\n", host, port);

  context = redisConnect(host, port);
  if (context == NULL || context->err) {
    if (context) {
      printf("Redis Connection Error: %s\n", context->errstr);
      redisFree(context);
    } else {
      perror("Connection Error: can't allocate redis context\n");
      redisFree(context);
    }
    return -1;
  }

  repsheet_members = redisCommand(context, "SMEMBERS repsheet");

  for (i = 0; i < repsheet_members->elements; i++) {
    reply = redisCommand(context, "EXISTS %s", repsheet_members->element[i]->str);
    if (reply->integer == 0) {
      printf("  Removing %s\n", repsheet_members->element[i]->str);
      freeReplyObject(redisCommand(context, "SREM repsheet %s", repsheet_members->element[i]->str));
    }
    freeReplyObject(reply);
  }

  freeReplyObject(repsheet_members);
  redisFree(context);

  printf("Repsheet cleanup complete\n");

  return 0;
}
