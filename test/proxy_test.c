#include "../src/proxy.h"
#include "test_suite.h"

START_TEST(returns_null_when_headers_are_null)
{
  fail_unless(process_headers(NULL) == NULL);
}
END_TEST

START_TEST(processes_a_single_address) {
  fail_unless(strcmp("192.168.1.100", process_headers("192.168.1.100")) == 0);
}
END_TEST

Suite *make_proxy_suite(void) {
  Suite *suite = suite_create("Proxy");

  TCase *tc_proxy = tcase_create("Standard");
  tcase_add_test(tc_proxy, returns_null_when_headers_are_null);
  tcase_add_test(tc_proxy, processes_a_single_address);
  suite_add_tcase(suite, tc_proxy);

  return suite;
}
