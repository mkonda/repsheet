#include "../src/repsheet.h"
#include "test_suite.h"

redisContext *context;
redisReply * reply;

void setup(void)
{
  context = redisConnect("localhost", 6379);

  if (context == NULL || context->err) {
    ck_abort_msg("Could not connect to Redis");
  }
}

void teardown(void)
{
  freeReplyObject(reply);
  redisFree(context);
}

START_TEST(handles_null_values)
{
  repsheet_record(context, NULL, NULL, NULL, NULL, NULL, "1.1.1.1", 100);

  reply = redisCommand(context, "LRANGE 1.1.1.1:requests 0 0");
  ck_assert_str_eq(reply->element[0]->str, "-, -, -, -, -");
}
END_TEST

START_TEST(properly_records_timestamp)
{
  repsheet_record(context, "12345", NULL, NULL, NULL, NULL, "1.1.1.1", 100);

  reply = redisCommand(context, "LRANGE 1.1.1.1:requests 0 0");
  ck_assert_str_eq(reply->element[0]->str, "12345, -, -, -, -");
}
END_TEST

START_TEST(properly_records_user_agent)
{
  repsheet_record(context, NULL, "A User Agent", NULL, NULL, NULL, "1.1.1.1", 100);

  reply = redisCommand(context, "LRANGE 1.1.1.1:requests 0 0");
  ck_assert_str_eq(reply->element[0]->str, "-, A User Agent, -, -, -");
}
END_TEST

START_TEST(properly_records_http_method)
{
  repsheet_record(context, NULL, NULL, "GET", NULL, NULL, "1.1.1.1", 100);

  reply = redisCommand(context, "LRANGE 1.1.1.1:requests 0 0");
  ck_assert_str_eq(reply->element[0]->str, "-, -, GET, -, -");
}
END_TEST

START_TEST(properly_records_uri)
{
  repsheet_record(context, NULL, NULL, NULL, "/", NULL, "1.1.1.1", 100);

  reply = redisCommand(context, "LRANGE 1.1.1.1:requests 0 0");
  ck_assert_str_eq(reply->element[0]->str, "-, -, -, /, -");
}
END_TEST

START_TEST(properly_records_arguments)
{
  repsheet_record(context, NULL, NULL, NULL, NULL, "foo=bar", "1.1.1.1", 100);

  reply = redisCommand(context, "LRANGE 1.1.1.1:requests 0 0");
  ck_assert_str_eq(reply->element[0]->str, "-, -, -, -, foo=bar");
}
END_TEST

Suite *make_repsheet_suite(void) {
  Suite *suite = suite_create("Repsheet");

  TCase *tc_repsheet = tcase_create("repsheet_record");
  tcase_add_checked_fixture(tc_repsheet, setup, teardown);
  tcase_add_test(tc_repsheet, handles_null_values);
  tcase_add_test(tc_repsheet, properly_records_timestamp);
  tcase_add_test(tc_repsheet, properly_records_user_agent);
  tcase_add_test(tc_repsheet, properly_records_http_method);
  tcase_add_test(tc_repsheet, properly_records_arguments);
  tcase_add_test(tc_repsheet, properly_records_uri);
  suite_add_tcase(suite, tc_repsheet);

  return suite;
}
