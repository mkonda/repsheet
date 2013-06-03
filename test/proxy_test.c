#include "../src/proxy.h"
#include "test_suite.h"

START_TEST(returns_null_when_headers_are_null)
{
  fail_unless(process_headers(NULL) == NULL);
}
END_TEST

START_TEST(processes_a_single_address) {
  ck_assert_str_eq(process_headers("192.168.1.100"), "192.168.1.100");
}
END_TEST

START_TEST(extract_only_the_first_ip_address)
{
  ck_assert_str_eq(process_headers("8.8.8.8 12.34.56.78, 212.23.230.15"), "8.8.8.8");
}
END_TEST

START_TEST(ignores_user_generated_noise)
{
  ck_assert_str_eq(process_headers("\\x5000 8.8.8.8, 12.23.45.67"), "8.8.8.8");
  ck_assert_str_eq(process_headers("This is not an IP address 8.8.8.8, 12.23.45.67"), "8.8.8.8");
  ck_assert_str_eq(process_headers("999.999.999.999, 8.8.8.8, 12.23.45.67"), "8.8.8.8");
}
END_TEST

Suite *make_proxy_suite(void) {
  Suite *suite = suite_create("Proxy");

  TCase *tc_proxy = tcase_create("Standard");
  tcase_add_test(tc_proxy, returns_null_when_headers_are_null);
  tcase_add_test(tc_proxy, processes_a_single_address);
  tcase_add_test(tc_proxy, extract_only_the_first_ip_address);
  suite_add_tcase(suite, tc_proxy);

  TCase *tc_proxy_malicious = tcase_create("Malicious");
  tcase_add_test(tc_proxy_malicious, ignores_user_generated_noise);
  suite_add_tcase(suite, tc_proxy_malicious);

  return suite;
}
