AC_INIT([dbmig command-line client], [0.0.1], [bugs@xsco.org], [dbmig])
AM_INIT_AUTOMAKE([-Wall])

AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CXX
AM_PROG_CC_C_O
AC_LANG([C++])
AX_CXX_COMPILE_STDCXX_11([noext],[mandatory])

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

