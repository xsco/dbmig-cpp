#!/bin/sh
./cleanup.sh

#aclocal
#autoheader
#autoconf
#automake --copy --add-missing --foreign
autoreconf -i

