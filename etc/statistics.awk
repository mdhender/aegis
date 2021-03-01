#
#	aegis - project change supervisor
#	Copyright (C) 1999, 2006-2008 Peter Miller
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
function spiffy(s) {
	tmp = s
	gsub(/_/, " ", tmp)
	an = split(tmp, a, " ")
	result = ""
	for (j = 1; j <= an; ++j)
	{
		tmp = a[j]
		tmp = toupper(substr(tmp, 1, 1)) tolower(substr(tmp, 2, 99))
		if (length(result) && length(tmp))
			result = result " "
		result = result tmp
	}
	return result
}

/metrics =/ {
	++nfiles
	next
}
/name.*value/ {
	name = $4
	value = 0 + $7
	sub(/^"/, "", name)
	sub(/".*/, "", name)
	summary[name] += value
	if (min[name] == 0 || min[name] > value)
		min[name] = value
	if (max[name] < value)
		max[name] = value
}
END {
	if (nfiles == 0)
		per_file = 1
	else
		per_file = 1.0 / nfiles
	print ".TS"
	print "tab(;);"
	print "c c c c c c"
	print "c c c c c c"
	print "l n n n n n."
	print "Description;Total;Minimum;Average;Maximum"
	print ";;Per File;Per File;Per File"
	print "_"
	for (name in summary)
	{
		printf("%s;%d;%d;%4.2f;%d\n",
			spiffy(name),
			summary[name],
			min[name],
			summary[name] * per_file,
			max[name]);
	}
	print ".TE"
	print ".sp 0.5"
	printf "Statistics collected over %d source files.\n", nfiles
	n = ENVIRON["ALL_FILES"] + 0
	if (n > nfiles)
	{
		printf							\
		(							\
			"The remaining %d source files (%d%%)\n",	\
			(n - nfiles), 100 * (n - nfiles)/n		\
		);
		print "are not covered here."
	}
}
