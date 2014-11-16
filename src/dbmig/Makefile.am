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
dbmig_LDADD = ../libdbmig/libdbmig.la \
	-lboost_system \
	-lboost_program_options

