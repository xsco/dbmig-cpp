noinst_LTLIBRARIES = libdbmig.la

# Source files.
libdbmig_la_SOURCES = \
	db_specific.cpp db_specific.hpp \
	changelog_table.cpp changelog_table.hpp \
	changelog.cpp \
	script_action.cpp \
	script_dir.cpp semver_compare.hpp \
	check.cpp \
	migrate.cpp \
	repository.cpp \
	time.cpp time.hpp \
	hash.hpp \
	getline.hpp \
	statement_buffer.hpp
include_HEADERS = \
	semantic_version.hpp \
	exception.hpp \
	changelog_entry.hpp changelog.hpp \
	diff.hpp \
	script_action.hpp \
	script_dir.hpp \
	script_stream.hpp \
	check.hpp \
	migrate.hpp \
	repository.hpp


# Compiler flags.
libdbmig_la_CPPFLAGS = \
	-Werror -Wall

# Additional libraries.
libdbmig_la_LIBADD = \
	-lsoci_core \
	-lcryptopp \
	-lboost_system \
	-lboost_filesystem \
	-lboost_regex \
	-lnowide

