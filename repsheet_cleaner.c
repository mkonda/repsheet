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

