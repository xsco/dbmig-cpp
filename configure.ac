AC_INIT([dbmig command-line client], [0.0.1], [bugs@xsco.org], [dbmig])
#AM_SILENT_RULES([yes])
AM_INIT_AUTOMAKE([-Wall])

AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CXX
AM_PROG_CC_C_O
AC_LANG([C++])

# Require C++11
AX_CXX_COMPILE_STDCXX_11([noext],[mandatory])

# Check some dependencies.
AC_CHECK_HEADER([boost/program_options.hpp])
AC_CHECK_HEADER([boost/filesystem.hpp])
AC_CHECK_HEADER([boost/regex.hpp])
AC_CHECK_HEADER([boost/algorithm/string/trim.hpp])
AC_CHECK_HEADER([nowide/convert.hpp])
AC_CHECK_HEADER([soci/soci.h])
PKG_CHECK_MODULES([libcryptopp], [libcrypto++ >= 5.6.0])

AM_PROG_AR
LT_INIT

# Use config.h
AC_CONFIG_HEADERS([config.h])

# Makefiles
AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/libdbmig/Makefile
	src/dbmig/Makefile
	src/libdbmigtest/Makefile
])


AC_OUTPUT

