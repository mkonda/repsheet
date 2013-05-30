TESTS = test_runner
check_PROGRAMS = $(TESTS)
test_runner_SOURCES = proxy_test.c test_runner.c $(top_builddir)/src/proxy.c $(top_builddir)/src/proxy.h
test_runner_CFLAGS = $(CHECK_CFLAGS) -lpcre
test_runner_LDADD = $(CHECK_LIBS) -lpcre
