//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2002, 2004-2006, 2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/ac/stdarg.h>
#include <common/ac/string.h>
#include <common/ac/sys/clu.h>


#if !HAVE_CLU_INFO

int
clu_info(int n, ...)
{
	va_list		ap;

	//
	// In order to be maximally portable, you must actually pull
	// the arguments out of the variable argument list.  On some
	// architectures, you can't return until at >> run-time << you
	// know where on the stack the return address can be found.
	// That's why there is a va_end() macro defined in the standard.
	//
	va_start(ap, n);
	switch (n)
	{
	case CLU_INFO_MY_ID:
		{
			memberid_t	*np;

			np = va_arg(ap, memberid_t *);
			*np = 0;
		}
		break;

	case CLU_INFO_NODENAME_BY_ID:
		{
			memberid_t	id;
			char		*ptr;
			size_t		len;

			id = va_arg(ap, memberid_t);
			ptr = va_arg(ap, char *);
			len = va_arg(ap, size_t);
			memset(ptr, 0, len);
			ptr[0] = '0' + id;
		}
		break;
	}
	va_end(ap);
	return -1;
}

#endif
