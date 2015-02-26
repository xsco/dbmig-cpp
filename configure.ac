AC_INIT([dbmig command-line client], [0.0.1], [bugs@xsco.org], [dbmig])
AM_SILENT_RULES([yes])
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

# Some OS-detection and suffix-naming of libraries accordingly.
case $host in
	i?86*mingw32*)
		os=win32;
		soci_suffix="_3_2";
		boost_suffix="-mt";
		ldflag_static_libgcc="-static-libgcc";
		ldflag_static_libstdcpp="-static-libstdc++";
		;;
	i?86*cygwin*)
		os=win32;
		soci_suffix="_3_2";
		boost_suffix="-mt";
		ldflag_static_libgcc="-static-libgcc";
		ldflag_static_libstdcpp="-static-libstdc++";
		;;
	i?86*linux*)
		os=linux;
		soci_suffix="";
		boost_suffix="";
		ldflag_static_libgcc="";
		ldflag_static_libstdcpp="";
		;;
	x86_64*linux*)
		os=linux;
		soci_suffix="";
		boost_suffix="";
		ldflag_static_libgcc="";
		ldflag_static_libstdcpp="";
		;;
	*)
		AC_MSG_ERROR([Unsupported system architecture: $host]);
		;;
esac

# Host-dependent LDFLAGS for possible static linkage.
AC_SUBST([LDFLAG_STATIC_LIBGCC], [$ldflag_static_libgcc])
AC_SUBST([LDFLAG_STATIC_LIBSTDCPP], [$ldflag_static_libstdcpp])

# Library names.
AC_SUBST([LIB_NOWIDE], [nowide])
AC_SUBST([LIB_SOCI_CORE], [soci_core$soci_suffix])
AC_SUBST([LIB_CRYPTOPP], [cryptopp])
AC_SUBST([LIB_BOOST_SYSTEM], [boost_system$boost_suffix])
AC_SUBST([LIB_BOOST_PROGRAM_OPTIONS], [boost_program_options$boost_suffix])
AC_SUBST([LIB_BOOST_FILESYSTEM], [boost_filesystem$boost_suffix])
AC_SUBST([LIB_BOOST_REGEX], [boost_regex$boost_suffix])
AC_SUBST([LIB_BOOST_UNIT_TEST_FRAMEWORK], [boost_unit_test_framework$boost_suffix])


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

