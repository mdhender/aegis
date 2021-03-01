#!/bin/sh
#
# aegis - project change supervisor
# Copyright (C) 2005-2008 Peter Miller
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or (at
# your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#

#
# where to get the distribution tarball from
#
src_tgz=$1

#
# Where to write the sourceforge tarball to
#
dst_tgz=$2

#
# The sourceforge configuration prefix
#
PREFIX=/home/groups/a/ae/aegis

set -e
set -x
rm -rf check-tarball-tmp
mkdir check-tarball-tmp
cd check-tarball-tmp
tar xzf $src_tgz
cd aegis-*
./configure --prefix=$PREFIX --with-no-aegis-configured
sed -e '/CXXFLAGS *=/s|$| -DSOURCE_FORGE_HACK|' \
    -e '/LDFLAGS *=/s|$| -static|' \
    -e 's|-lcurl||g' \
    -e 's|-lgssapi_krb5||g' \
    -e '/^#STATIC_LIBS *=/s|^#||' \
    Makefile > Makefile.new
mv Makefile.new Makefile

sed  \
    -e '/define HAVE_CURL/d' \
    -e '/define HAVE_LIBCURL/d' \
    < common/config.h > common/config.h.new
mv common/config.h.new common/config.h

grep 'CXXFLAGS *=' Makefile
grep 'LDFLAGS *=' Makefile
make
du -s -h
mkdir -p $PREFIX
make install
cd $PREFIX
mkdir -p -m755 cgi-bin
# we exclude almost everything
for f in ` ls bin `
do
    case $f in
    aedist | \
    aediff | \
    aegis | \
    aeget | \
    aepatch | \
    aetar | \
    aerevml | \
    aesvt )
        # keep these
        ;;
    *)
        # get rid of everything else
        rm bin/$f
        ;;
    esac
done

rm -r com lib etc
rm -r man || true
mkdir -p -m755 share-new/en
mv share/en/html share-new/en/.
mv share/icon share-new/.
rm -r share
mv share-new share
strip bin/* || true

sed "s|@PREFIX@|$PREFIX|" > cgi-bin/aeget << 'fubar'
#!/bin/sh
cmd=`basename $0`
bin=@PREFIX@/bin
PATH=${bin}:$PATH
export PATH
exec $bin/$cmd "$@"
fubar
chmod a+rx cgi-bin/aeget

sed "s|@PREFIX@|$PREFIX|" > cgi-bin/aegis.cgi << 'fubar'
#!/bin/sh
cmd=`basename $0`
bin=@PREFIX@/bin
PATH=${bin}:$PATH
export PATH
exec $bin/$cmd "$@"
fubar
chmod a+rx cgi-bin/aegis.cgi

tar czf $dst_tgz bin cgi-bin share
ls -lh $dst_tgz
