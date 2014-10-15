AC_PREREQ([2.68])
AC_INIT([testgeard], [0.2], [], [testgeard], [http://github.com/testgear/testgeard])
AC_CONFIG_HEADERS([src/include/config.h])
AM_INIT_AUTOMAKE([1.11 foreign dist-xz no-dist-gzip -Wall -Werror])
AM_SILENT_RULES([yes])
AC_PROG_CC
AM_PROG_AR
AC_ENABLE_STATIC
AC_DISABLE_SHARED

LT_PREREQ([2.2])
LT_INIT

AC_CONFIG_MACRO_DIR([m4])
AC_LANG([C])
AC_PROG_INSTALL
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([man/Makefile])
AC_OUTPUT
