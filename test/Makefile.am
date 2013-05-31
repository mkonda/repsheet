TESTS = test_runner
check_PROGRAMS = $(TESTS)
test_runner_SOURCES = proxy_test.c \
                      mod_security_test.c \
                      test_runner.c \
                      $(top_builddir)/src/proxy.c \
                      $(top_builddir)/src/proxy.h \
                      $(top_builddir)/src/mod_security.c \
                      $(top_builddir)/src/mod_security.h
test_runner_CFLAGS = $(CHECK_CFLAGS)
test_runner_LDADD = $(CHECK_LIBS) -lpcre
