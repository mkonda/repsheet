#include "test_suite.h"

START_TEST(things_are_properly_wired) {
  fail_unless(1 == 1);
}
END_TEST

Suite *make_sample_suite(void) {
  Suite *suite = suite_create("Sample");

  TCase *tc_sample = tcase_create("Truthy");
  tcase_add_test(tc_sample, things_are_properly_wired);
  suite_add_tcase(suite, tc_sample);

  return suite;
}
