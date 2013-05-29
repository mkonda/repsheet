AC_PREREQ(2.61)
AC_INIT([repsheet], [0.0.2], [aaron@aaronbedra.com])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([-Wall foreign])
AC_CONFIG_MACRO_DIR([m4])
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

AC_PROG_CC
AC_PROG_LIBTOOL

AC_CHECK_LIB([apr-1], [apr_pool_pre_cleanup_register], [], [AC_MSG_ERROR([apr_pool_pre_cleanup_register was not found in libapr-1])])

AC_CHECK_FILE([/usr/local/apache2/include/httpd.h], [APACHE_INCLUDE_DIR="/usr/local/apache2/include"], [
    AC_CHECK_FILE([/usr/include/httpd/httpd.h], [APACHE_INCLUDE_DIR="/usr/include/httpd"], [
        AC_CHECK_FILE([/usr/include/apache2/httpd.h], [APACHE_INCLUDE_DIR="/usr/include/apache2"], [
            AC_MSG_ERROR([Could not locate Apache include directory])
        ])
    ])
])
AC_SUBST([APACHE_INCLUDE_DIR])

AC_CHECK_FILE([/usr/local/apache2/bin/apxs], [APXS="/usr/local/apache2/bin/apxs"], [
    AC_CHECK_FILE([/usr/sbin/apxs], [APXS="/usr/sbin/apxs"], [
        AC_CHECK_FILE([/usr/bin/apxs2], [APXS="/usr/bin/apxs2"], [
            AC_MSG_ERROR([Could not locate Apache apxs binary])
        ])
    ])
])
AC_SUBST([APXS])

AC_CONFIG_FILES([
    Makefile
    src/Makefile
])

AC_OUTPUT
