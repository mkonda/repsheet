#include "../src/repsheet.h"
#include "test_suite.h"

redisContext *context;
redisReply *reply;

void setup(void)
{
  context = redisConnect("localhost", 6379);

  if (context == NULL || context->err) {
    ck_abort_msg("Could not connect to Redis");
  }
}

void teardown(void)
{
  freeReplyObject(redisCommand(context, "flushdb"));
  if (!reply == NULL) {
    freeReplyObject(reply);
  }
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

START_TEST(repsheet_ip_lookup_returns_allow_when_not_on_repsheet_or_blacklist)
{
  ck_assert_int_eq(repsheet_ip_lookup(context, "1.1.1.1"), ALLOW);
}
END_TEST

START_TEST(repsheet_ip_lookup_returns_allow_when_on_the_whitelist)
{
  freeReplyObject(redisCommand(context, "SET 1.1.1.1:repsheet:blacklist true"));
  freeReplyObject(redisCommand(context, "SET 1.1.1.1:repsheet:whitelist true"));

  ck_assert_int_eq(repsheet_ip_lookup(context, "1.1.1.1"), ALLOW);
}
END_TEST

START_TEST(repsheet_ip_lookup_returns_notify_when_on_the_repsheet)
{
  freeReplyObject(redisCommand(context, "SET 1.1.1.1:repsheet true"));

  ck_assert_int_eq(repsheet_ip_lookup(context, "1.1.1.1"), NOTIFY);
}
END_TEST

START_TEST(repsheet_ip_lookup_returns_block_when_on_the_blacklist)
{
  freeReplyObject(redisCommand(context, "SET 1.1.1.1:repsheet:blacklist true"));

  ck_assert_int_eq(repsheet_ip_lookup(context, "1.1.1.1"), BLOCK);
}
END_TEST

Suite *make_repsheet_suite(void) {
  Suite *suite = suite_create("librepsheet");

  TCase *tc_record = tcase_create("repsheet_record");
  tcase_add_checked_fixture(tc_record, setup, teardown);
  tcase_add_test(tc_record, handles_null_values);
  tcase_add_test(tc_record, properly_records_timestamp);
  tcase_add_test(tc_record, properly_records_user_agent);
  tcase_add_test(tc_record, properly_records_http_method);
  tcase_add_test(tc_record, properly_records_arguments);
  tcase_add_test(tc_record, properly_records_uri);
  suite_add_tcase(suite, tc_record);

  TCase *tc_ip_lookup = tcase_create("repsheet_ip_lookup");
  tcase_add_checked_fixture(tc_ip_lookup, setup, teardown);
  tcase_add_test(tc_ip_lookup, repsheet_ip_lookup_returns_allow_when_not_on_repsheet_or_blacklist);
  tcase_add_test(tc_ip_lookup, repsheet_ip_lookup_returns_allow_when_on_the_whitelist);
  tcase_add_test(tc_ip_lookup, repsheet_ip_lookup_returns_notify_when_on_the_repsheet);
  tcase_add_test(tc_ip_lookup, repsheet_ip_lookup_returns_block_when_on_the_blacklist);
  suite_add_tcase(suite, tc_ip_lookup);

  return suite;
}
