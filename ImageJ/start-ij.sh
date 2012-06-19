#!/bin/sh

IJDIR="$(dirname "$0")"

java -Djava.library.path=$IJDIR/lib/linux32 -Dplugins.dir=$IJDIR -jar $IJDIR/ij.jar "$@"
