bin_PROGRAMS = dbmig

# Source files.
dbmig_SOURCES = dbmig.cpp \
	console_util.cpp console_util.hpp \
	services.hpp show.cpp check.cpp override_version.cpp migrate.cpp

# Compiler flags.
dbmig_CPPFLAGS = \
	-I../libdbmig \
	-Werror -Wall

# Linker flags.
dbmig_LDFLAGS = -s $(LDFLAG_STATIC_LIBGCC) $(LDFLAG_STATIC_LIBSTDCPP)

# Additional libraries for the linker.
dbmig_LDADD = ../libdbmig/libdbmig.la \
	-l$(LIB_NOWIDE) \
	-l$(LIB_BOOST_SYSTEM) \
	-l$(LIB_BOOST_PROGRAM_OPTIONS)

