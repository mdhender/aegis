#!/bin/sh
#
# MANIFEST: shell script to install section 5 manual entries
#
case $# in
1)
	;;
*)
	echo "usage: $0 somewhere/man5" 1>&2
	exit 1
	;;
esac

set -e
cd man5
for file in *.5
do
	echo "soelim $file > $1/$file"
	soelim $file > $1/$file
done
exit 0
