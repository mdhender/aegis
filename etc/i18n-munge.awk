#
#	aegis - project change supervisor
#	Copyright (C) 2004, 2006-2008 Peter Miller
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 3 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program. If not, see
#	<http://www.gnu.org/licenses/>.
#
# This awk script munges the POT file into something that the
# Translation Project's tools can cope with.  Aparrently it isn't enough
# that the file is in a correct format, it has to be in the default
# (empty msgstr) format coming out of xgettext, and unindented, too.
#
# The English translation is thus shifted into the leading comment for
# translator's reference.
#
# The leading header comment is also mangled to add the "optional"
# Project-Id-Version and POT-Creation-Date lines which their system
# can't seem to cope without.
#
function process()
{
    if (msgid_value != "")
    {
	if (msgid_value == "msgid \"\"")
	{
	    print msgid_value
	    print msgstr_value
	    vers = ENVIRON["VERSION"]
	    if (vers != "")
		print "\"Project-Id-Version: aegis " vers "\\n\""
	    print "\"POT-Creation-Date: " strftime() "\\n\""
	}
	else
	{
	    print "# For consistent translation, here is the English text:"
	    print hash_msgstr_value
	    print "# Please translate the English msgstr, not the msgid."
	    print msgid_value
	    print "msgstr \"\""
	}
    }
    msgid = 0
    msgid_value = ""
    msgstr = 0
    msgstr_value = ""
    hash_msgstr_value = ""
}

/^msgid/ {
    msgid = 1
    msgid_value = $0
    next
}
/^msgstr/ {
    msgid = 0
    msgstr = 1
    msgstr_value = $0
    hash_msgstr_value = "#  " $0
    next
}
/^[ 	]*".*"$/ {
    if (msgid)
	msgid_value = msgid_value "\n" $0
    if (msgstr)
    {
	msgstr_value = msgstr_value "\n" $0
	hash_msgstr_value = hash_msgstr_value "\n#  " $0
    }
    next
}
/^[ 	]*$/ {
    process()
    print
    next
}
{
    print
}
END {
    process()
}
