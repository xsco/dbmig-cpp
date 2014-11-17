#!/bin/sh
./cleanup.sh
echo "(This file was copied from README.md - don't edit this file directly)" > README
cat README.md >> README
autoreconf -i

