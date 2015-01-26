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
dbmig_LDFLAGS = -s
dbmig_LDFLAGS += -static-libgcc -static-libstdc++

# Additional libraries for the linker.
#dbmig_LDADD = ../libdbmig/libdbmig.la \
#	-lboost_system \
#	-lboost_program_options \
#	-lnowide

dbmig_LDADD = ../libdbmig/libdbmig.la \
	-lboost_system-mt \
	-lboost_program_options-mt \
	-lnowide

