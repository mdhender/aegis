/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994 Peter Miller.
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * MANIFEST: functions to implement the builtin sprintf function
 */

#include <stdio.h>
#include <ac/string.h>

#include <aer/expr.h>
#include <aer/func/sprintf.h>
#include <aer/value/error.h>
#include <aer/value/integer.h>
#include <aer/value/real.h>
#include <aer/value/string.h>
#include <error.h>
#include <mem.h>
#include <str.h>

/*
 * size to grow memory by
 */
#define QUANTUM 200

/*
 * maximum width for numbers
 */
#define MAX_WIDTH (QUANTUM - 1)

/*
 * the buffer for storing results
 */
static size_t	tmplen;
static size_t	length;
static char	*tmp;

/*
 * infor for cranking through the arguments
 */
static size_t	ac;
static size_t	ai;
static rpt_value_ty **av;


/*
 * NAME
 *	bigger - grow dynamic memory buffer
 *
 * SYNOPSIS
 *	int bigger(void);
 *
 * DESCRIPTION
 *	The bigger function is used to grow the dynamic memory buffer
 *	used by vmprintf to store the formatting results.
 *	The buffer is increased by QUANTUM bytes.
 *
 * RETURNS
 *	int; zero if failed to realloc memory, non-zero if successful.
 *
 * CAVEATS
 *	The existing buffer is still valid after failure.
 */

static void bigger _((void));

static void
bigger()
{
	tmplen += QUANTUM;
	tmp = mem_change_size(tmp, tmplen);
}


/*
 * NAME
 *	build fake - construct formatting specifier string
 *
 * SYNOPSIS
 *	void build_fake(char *fake, int flag, int width, int prec, int qual,
 *		int spec);
 *
 * DESCRIPTION
 *	The build_fake function is used to construct a format
 *	specification string from the arguments presented.  This is
 *	used to guarantee exact replication of sprintf behaviour.
 *
 * ARGUMENTS
 *	fake	- buffer to store results
 *	flag	- the flag specified (zero if not)
 *	width	- the width specified (zero if not)
 *	prec	- the precision specified (zero if not)
 *	qual	- the qualifier specified (zero if not)
 *	spec	- the formatting specifier specified
 */

static void build_fake _((char *fake, int flag, int width, int precision,
	int qualifier, int specifier));

static void
build_fake(fake, flag, width, precision, qualifier, specifier)
	char		*fake;
	int		flag;
	int		width;
	int		precision;
	int		qualifier;
	int		specifier;
{
	char		*fp;

	fp = fake;
	*fp++ = '%';
	if (flag)
		*fp++ = flag;
	if (width > 0)
	{
		sprintf(fp, "%d", width);
		fp += strlen(fp);
	}
	*fp++ = '.';
	sprintf(fp, "%d", precision);
	fp += strlen(fp);
	if (qualifier)
		*fp++ = qualifier;
	*fp++ = specifier;
	*fp = 0;
}


static int verify _((rpt_expr_ty *));

static int
verify(ep)
	rpt_expr_ty	*ep;
{
	return (ep->nchild >= 1);
}


static rpt_value_ty *get_arg _((void));

static rpt_value_ty *
get_arg()
{
	if (ai >= ac)
		return rpt_value_error("sprintf: too few arguments");
	return rpt_value_copy(av[ai++]);
}


static rpt_value_ty *get_arg_string _((void));

static rpt_value_ty *
get_arg_string()
{
	rpt_value_ty	*vp1;
	rpt_value_ty	*vp2;

	vp1 = get_arg();
	if (vp1->method->type == rpt_value_type_error)
		return vp1;
	vp2 = rpt_value_stringize(vp1);
	rpt_value_free(vp1);
	if (vp2->method->type == rpt_value_type_string)
		return vp2;
	vp1 =
		rpt_value_error
		(
		  "sprintf: argument %ld: string value required (was given %s)",
			vp2->method->name
		);
	rpt_value_free(vp2);
	return vp1;
}


static rpt_value_ty *get_arg_integer _((int));

static rpt_value_ty *
get_arg_integer(real_ok)
	int		real_ok;
{
	rpt_value_ty	*vp1;
	rpt_value_ty	*vp2;

	vp1 = get_arg();
	if (vp1->method->type == rpt_value_type_error)
		return vp1;
	vp2 = rpt_value_arithmetic(vp1);
	rpt_value_free(vp1);
	if (vp2->method->type == rpt_value_type_integer)
		return vp2;
	if (real_ok && vp2->method->type == rpt_value_type_real)
	{
		vp1 = rpt_value_integerize(vp2);
		if (vp1->method->type == rpt_value_type_integer)
		{
			rpt_value_free(vp2);
			return vp1;
		}
		/* not representable as a long */
		rpt_value_free(vp1);
	}
	vp1 =
		rpt_value_error
		(
		 "sprintf: argument %ld: integer value required (was given %s)",
			vp2->method->name
		);
	rpt_value_free(vp2);
	return vp1;
}


static rpt_value_ty *get_arg_real _((int));

static rpt_value_ty *
get_arg_real(integer_ok)
	int		integer_ok;
{
	rpt_value_ty	*vp1;
	rpt_value_ty	*vp2;

	vp1 = get_arg();
	if (vp1->method->type == rpt_value_type_error)
		return vp1;
	vp2 = rpt_value_arithmetic(vp1);
	rpt_value_free(vp1);
	if (vp2->method->type == rpt_value_type_real)
		return vp2;
	if (integer_ok && vp2->method->type == rpt_value_type_integer)
	{
		vp1 = rpt_value_realize(vp2);
		rpt_value_free(vp2);
		return vp1;
	}
	vp1 =
		rpt_value_error
		(
		    "sprintf: argument %ld: real value required (was given %s)",
			vp2->method->name
		);
	rpt_value_free(vp2);
	return vp1;
}


/*
 * NAME
 *	run - build a formatted string in dynamic memory
 *
 * SYNOPSIS
 *	char *run(rpt_expr_ty *ep, size_t argc, rpt_value_ty *argv);
 *
 * DESCRIPTION
 *	The run function is used to build a formatted string
 *	in memory.  It understands all of the ANSI standard sprintf
 *	formatting directives.  Except, "%n" is not implemented.
 *
 * ARGUMENTS
 *	ep	- expr calling the function
 *	argc	- number of arguments
 *	argv	- values of the arguments
 *
 * RETURNS
 *	rpt_value_ty *; pointer to value containing formatted string
 */

static rpt_value_ty *run _((rpt_expr_ty *, size_t, rpt_value_ty **));

static rpt_value_ty *
run(ep, argc, argv)
	rpt_expr_ty	*ep;
	size_t		argc;
	rpt_value_ty	**argv;
{
	char		*fmt;
	int		width;
	int		width_set;
	int		prec;
	int		prec_set;
	int		c;
	string_ty	*s;
	int		qualifier;
	int		flag;
	char		fake[QUANTUM - 1];
	rpt_value_ty	*result;
	rpt_value_ty	*fmt_vp;

	/*
	 * Build the result string in a temporary buffer.
	 * Grow the temporary buffer as necessary.
	 *
	 * It is important to only make one pass across the variable argument
	 * list.  Behaviour is undefined for more than one pass.
	 */
	if (!tmplen)
	{
		tmplen = 500;
		tmp = mem_alloc(tmplen);
	}
	length = 0;

	/*
	 * get the format string
	 */
	ac = argc;
	ai = 0;
	av = argv;
	fmt_vp = get_arg_string();
	if (fmt_vp->method->type == rpt_value_type_error)
	{
		fmt_vp = 0;
		result = fmt_vp;
		goto done;
	}
	assert(fmt_vp->method->type == rpt_value_type_string);
	fmt = rpt_value_string_query(fmt_vp)->str_text;

	while (*fmt)
	{
		c = *fmt++;
		if (c != '%')
		{
			normal:
			if (length >= tmplen)
				bigger();
			tmp[length++] = c;
			continue;
		}
		c = *fmt++;

		/*
		 * get optional flag
		 */
		switch (c)
		{
		case '+':
		case '-':
		case '#':
		case '0':
		case ' ':
			flag = c;
			c = *fmt++;
			break;

		default:
			flag = 0;
			break;
		}

		/*
		 * get optional width
		 */
		width = 0;
		width_set = 0;
		switch (c)
		{
		case '*':
			result = get_arg_integer(0);
			if (result->method->type == rpt_value_type_error)
				goto done;
			assert(result->method->type == rpt_value_type_integer);
			width = rpt_value_integer_query(result);
			rpt_value_free(result);
			if (width < 0)
			{
				flag = '-';
				width = -width;
			}
			if (width > MAX_WIDTH)
			{
				result =
					rpt_value_error
					(
			      "sprintf: argument %ld: width of %ld is too wide",
						ai,
						width
					);
				goto done;
			}
			c = *fmt++;
			width_set = 1;
			break;
		
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			for (;;)
			{
				width = width * 10 + c - '0';
				c = *fmt++;
				switch (c)
				{
				default:
					break;

				case '0': case '1': case '2': case '3':
				case '4': case '5': case '6': case '7':
				case '8': case '9':
					continue;
				}
				break;
			}
			if (width > MAX_WIDTH)
			{
				result =
					rpt_value_error
					(
			      "sprintf: argument %ld: width of %ld is too wide",
						ai + 1,
						width
					);
				goto done;
			}
			width_set = 1;
			break;

		default:
			break;
		}

		/*
		 * get optional precision
		 */
		prec = 0;
		prec_set = 0;
		if (c == '.')
		{
			c = *fmt++;
			switch (c)
			{
			default:
				prec_set = 1;
				break;

			case '*':
				c = *fmt++;
				result = get_arg_integer(0);
				if (result->method->type == rpt_value_type_error)
					goto done;
				assert(result->method->type == rpt_value_type_integer);
				prec = rpt_value_integer_query(result);
				rpt_value_free(result);
				if (prec < 0 || prec > MAX_WIDTH)
				{
					result =
						rpt_value_error
						(
		    "sprintf: argument %ld: a precision of %ld is out of range",
							ai,
							prec
						);
					goto done;
				}
				prec_set = 1;
				break;

			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				for (;;)
				{
					prec = prec * 10 + c - '0';
					c = *fmt++;
					switch (c)
					{
					default:
						break;

					case '0': case '1': case '2': case '3':
					case '4': case '5': case '6': case '7':
					case '8': case '9':
						continue;
					}
					break;
				}
				if (prec > MAX_WIDTH)
				{
					result =
						rpt_value_error
						(
		    "sprintf: argument %ld: a precision of %ld is out of range",
							ai + 1,
							prec
						);
					goto done;
				}
				prec_set = 1;
				break;
			}
		}

		/*
		 * get the optional qualifier
		 */
		switch (c)
		{
		default:
			qualifier = 0;
			break;

		case 'l':
		case 'h':
		case 'L':
			qualifier = c;
			c = *fmt++;
			break;
		}

		/*
		 * get conversion specifier
		 */
		switch (c)
		{
		default:
			result =
				rpt_value_error
				(
				       "sprintf: unknown format specifier '%c'",
					c
				);
			goto done;

		case '%':
			goto normal;

		case 'c':
			{
				long	a;
				char	num[MAX_WIDTH + 1];
				size_t	len;

				result = get_arg_integer(1);
				if (result->method->type == rpt_value_type_error)
					goto done;
				assert(result->method->type == rpt_value_type_integer);
				a = rpt_value_integer_query(result);
				rpt_value_free(result);

				if (!prec_set)
					prec = 1;
				build_fake(fake, flag, width, prec, 0, c);
				sprintf(num, fake, a);
				len = strlen(num);
				assert(len < QUANTUM);
				if (length + len > tmplen)
					bigger();
				memcpy(tmp + length, num, len);
				length += len;
			}
			break;

		case 'd':
		case 'i':
			{
				long	a;
				char	num[MAX_WIDTH + 1];
				size_t	len;

				result = get_arg_integer(1);
				if (result->method->type == rpt_value_type_error)
					goto done;
				assert(result->method->type == rpt_value_type_integer);
				a = rpt_value_integer_query(result);
				rpt_value_free(result);

				if (!prec_set)
					prec = 1;
				build_fake(fake, flag, width, prec, 'l', c);
				sprintf(num, fake, a);
				len = strlen(num);
				assert(len < QUANTUM);
				if (length + len > tmplen)
					bigger();
				memcpy(tmp + length, num, len);
				length += len;
			}
			break;

		case 'e':
		case 'f':
		case 'g':
		case 'E':
		case 'F':
		case 'G':
			{
				double	a;
				char	num[MAX_WIDTH + 1];
				size_t	len;

				result = get_arg_real(1);
				if (result->method->type == rpt_value_type_error)
					goto done;
				assert(result->method->type == rpt_value_type_real);
				a = rpt_value_real_query(result);
				rpt_value_free(result);

				if (!prec_set)
					prec = 6;
				if (prec > MAX_WIDTH)
					prec = MAX_WIDTH;
				build_fake(fake, flag, width, prec, 0, c);
				sprintf(num, fake, a);
				len = strlen(num);
				assert(len < QUANTUM);
				if (length + len > tmplen)
					bigger();
				memcpy(tmp + length, num, len);
				length += len;
			}
			break;

		case 'u':
		case 'o':
		case 'x':
		case 'X':
			{
				unsigned long	a;
				char		num[MAX_WIDTH + 1];
				size_t		len;

				result = get_arg_integer(1);
				if (result->method->type == rpt_value_type_error)
					goto done;
				assert(result->method->type == rpt_value_type_integer);
				a = rpt_value_integer_query(result);
				rpt_value_free(result);

				if (!prec_set)
					prec = 1;
				build_fake(fake, flag, width, prec, 'l', c);
				sprintf(num, fake, a);
				len = strlen(num);
				assert(len < QUANTUM);
				if (length + len > tmplen)
					bigger();
				memcpy(tmp + length, num, len);
				length += len;
			}
			break;

		case 's':
			{
				string_ty	*a;
				size_t		len;

				result = get_arg_string();
				if (result->method->type == rpt_value_type_error)
					goto done;
				assert(result->method->type == rpt_value_type_string);
				a = str_copy(rpt_value_string_query(result));
				rpt_value_free(result);

				len = a->str_length;
				if (!prec_set)
					prec = len;
				if (len < prec)
					prec = len;
				if (!width_set)
					width = prec;
				if (width < prec)
					width = prec;
				len = width;
				while (length + len > tmplen)
					bigger();
				if (flag != '-')
				{
					while (width > prec)
					{
						tmp[length++] = ' ';
						width--;
					}
				}
				memcpy(tmp + length, a->str_text, prec);
				length += prec;
				width -= prec;
				if (flag == '-')
				{
					while (width > 0)
					{
						tmp[length++] = ' ';
						width--;
					}
				}
				str_free(a);
			}
			break;
		}
	}

	if (ai < argc)
	{
		result =
			rpt_value_error
			(
		       "sprintf: too many arguments (%ld given, only %ld used)",
				(long)argc,
				(long)ai
			);
		goto done;
	}

	/*
	 * build return value
	 */
	s = str_n_from_c(tmp, length);
	result = rpt_value_string(s);
	str_free(s);

	/*
	 * clean up and go home
	 */
	done:
	if (fmt_vp)
		rpt_value_free(fmt_vp);
	if (result->method->type == rpt_value_type_error)
		rpt_expr_error(ep, "%S", rpt_value_error_query(result));
	return result;
}


rpt_func_ty rpt_func_sprintf =
{
	"sprintf",
	1, /* optimizable */
	verify,
	run
};
