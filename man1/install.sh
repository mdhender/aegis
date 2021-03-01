#!/bin/sh
#
# MANIFEST: shell script to install section 1 manual pages
#
case $# in
1)
	;;
*)
	echo "usage: $0 somewhere/man1" 1>&2
	exit 1
	;;
esac

set -e
cd man1
for file in *.1
do
	echo "soelim $file > $1/$file"
	soelim $file > $1/$file
done
exit 0
