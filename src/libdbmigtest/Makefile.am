check_PROGRAMS = repository_test script_dir_test script_stream_test diff_test
TESTS = $(check_PROGRAMS)

# Structure so that each .cpp class represents an individual test module.
# We'll name the programs after each .cpp file, one for each.
AM_DEFAULT_SOURCE_EXT = .cpp

repository_test_SOURCES = repository_test.cpp pair_special.hpp
script_dir_test_SOURCES = script_dir_test.cpp pair_special.hpp
script_stream_test_SOURCES = script_stream_test.cpp pair_special.hpp
diff_test_SOURCES = diff_test.cpp pair_special.hpp

# Compiler flags.
AM_CPPFLAGS = \
	-I../libdbmig \
	-Werror -Wall

# Linker flags.
LDADD = ../libdbmig/libdbmig.la \
	-lboost_system \
	-lboost_unit_test_framework

