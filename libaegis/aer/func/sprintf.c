/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994, 1995, 1996, 1999 Peter Miller;
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
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to implement the builtin sprintf function
 */

#include <ac/stdio.h>
#include <ac/string.h>

#include <aer/expr.h>
#include <aer/func/sprintf.h>
#include <aer/value/error.h>
#include <aer/value/integer.h>
#include <aer/value/real.h>
#include <aer/value/string.h>
#include <error.h>
#include <mem.h>
#include <sub.h>

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
 * info for cranking through the arguments
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


static rpt_value_ty *get_arg _((rpt_expr_ty *));

static rpt_value_ty *
get_arg(ep)
	rpt_expr_ty	*ep;
{
	if (ai >= ac)
	{
		sub_context_ty	*scp;
		string_ty	*s;
		rpt_value_ty	*result;

		scp = sub_context_new();
		sub_var_set_charstar(scp, "Function", "sprintf");
		s = subst_intl(scp, i18n("$function: too few arguments"));
		sub_context_delete(scp);
		result = rpt_value_error(ep->pos, s);
		str_free(s);
		return result;
	}
	return rpt_value_copy(av[ai++]);
}


static rpt_value_ty *get_arg_string _((rpt_expr_ty *));

static rpt_value_ty *
get_arg_string(ep)
	rpt_expr_ty	*ep;
{
	sub_context_ty	*scp;
	rpt_value_ty	*vp1;
	rpt_value_ty	*vp2;
	string_ty	*s;

	vp1 = get_arg(ep);
	assert(vp1->method->type != rpt_value_type_error);
	vp2 = rpt_value_stringize(vp1);
	rpt_value_free(vp1);
	if (vp2->method->type == rpt_value_type_string)
		return vp2;

	scp = sub_context_new();
	sub_var_set_charstar(scp, "Function", "sprintf");
	sub_var_set_charstar(scp, "Name", vp2->method->name);
	sub_var_set_long(scp, "Number", (long)ai);
	rpt_value_free(vp2);
	s =
		subst_intl
		(
			scp,
    i18n("$function: argument $number: string value required (was given $name)")
		);
	sub_context_delete(scp);
	vp1 = rpt_value_error(ep->pos, s);
	str_free(s);
	return vp1;
}


static rpt_value_ty *get_arg_integer _((rpt_expr_ty *, int));

static rpt_value_ty *
get_arg_integer(ep, real_ok)
	rpt_expr_ty	*ep;
	int		real_ok;
{
	sub_context_ty	*scp;
	rpt_value_ty	*vp1;
	rpt_value_ty	*vp2;
	string_ty	*s;

	vp1 = get_arg(ep);
	assert(vp1->method->type != rpt_value_type_error);
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

	scp = sub_context_new();
	sub_var_set_charstar(scp, "Function", "sprintf");
	sub_var_set_charstar(scp, "Name", vp2->method->name);
	sub_var_set_long(scp, "Number", (long)ai);
	rpt_value_free(vp2);
	s =
		subst_intl
		(
			scp,
   i18n("$function: argument $number: integer value required (was given $name)")
		);
	sub_context_delete(scp);
	vp1 = rpt_value_error(ep->pos, s);
	str_free(s);
	return vp1;
}


static rpt_value_ty *get_arg_real _((rpt_expr_ty *, int));

static rpt_value_ty *
get_arg_real(ep, integer_ok)
	rpt_expr_ty	*ep;
	int		integer_ok;
{
	sub_context_ty	*scp;
	rpt_value_ty	*vp1;
	rpt_value_ty	*vp2;
	string_ty	*s;

	vp1 = get_arg(ep);
	assert(vp1->method->type != rpt_value_type_error);
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

	scp = sub_context_new();
	sub_var_set_charstar(scp, "Function", "sprintf");
	sub_var_set_charstar(scp, "Name", vp2->method->name);
	sub_var_set_long(scp, "Number", (long)ai);
	rpt_value_free(vp2);
	s =
		subst_intl
		(
			scp,
      i18n("$function: argument $number: real value required (was given $name)")
		);
	sub_context_delete(scp);
	vp1 = rpt_value_error(ep->pos, s);
	str_free(s);
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
	sub_context_ty	*scp;
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
	fmt_vp = get_arg_string(ep);
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
			result = get_arg_integer(ep, 0);
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
				scp = sub_context_new();
				sub_var_set_charstar(scp, "Function", "sprintf");
				sub_var_set_long(scp, "Number", (long)ai);
				sub_var_set_long(scp, "Value", width);
				s =
					subst_intl
					(
						scp,
		  i18n("$function: argument $number: width $value out of range")
					);
				sub_context_delete(scp);
				result = rpt_value_error(ep->pos, s);
				str_free(s);
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
				scp = sub_context_new();
				sub_var_set_charstar(scp, "Function", "sprintf");
				sub_var_set_long(scp, "Number", (long)ai);
				sub_var_set_long(scp, "Value", width);
				s =
					subst_intl
					(
						scp,
		  i18n("$function: argument $number: width $value out of range")
					);
				sub_context_delete(scp);
				result = rpt_value_error(ep->pos, s);
				str_free(s);
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
				result = get_arg_integer(ep, 0);
				if (result->method->type == rpt_value_type_error)
					goto done;
				assert(result->method->type == rpt_value_type_integer);
				prec = rpt_value_integer_query(result);
				rpt_value_free(result);
				if (prec < 0 || prec > MAX_WIDTH)
				{
					scp = sub_context_new();
					sub_var_set_charstar(scp, "Function", "sprintf");
					sub_var_set_long(scp, "Number", (long)ai);
					sub_var_set_long(scp, "Value", prec);
					s =
						subst_intl
						(
							scp,
	i18n("$function: argument $number: precision of $value is out of range")
						);
					sub_context_delete(scp);
					result = rpt_value_error(ep->pos, s);
					str_free(s);
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
					scp = sub_context_new();
					sub_var_set_charstar(scp, "Function", "sprintf");
					sub_var_set_long(scp, "Number", (long)ai);
					sub_var_set_long(scp, "Value", prec);
					s =
						subst_intl
						(
							scp,
	i18n("$function: argument $number: precision of $value is out of range")
						);
					sub_context_delete(scp);
					result = rpt_value_error(ep->pos, s);
					str_free(s);
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
			scp = sub_context_new();
			sub_var_set_charstar(scp, "Function", "sprintf");
			sub_var_set_format(scp, "Name", "%c", c);
			s =
				subst_intl
				(
					scp,
			     i18n("$function: unknown format specifier '$name'")
				);
			sub_context_delete(scp);
			result = rpt_value_error(ep->pos, s);
			str_free(s);
			goto done;

		case '%':
			goto normal;

		case 'c':
			{
				long	a;
				char	num[MAX_WIDTH + 1];
				size_t	len;

				result = get_arg_integer(ep, 1);
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

				result = get_arg_integer(ep, 1);
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

				result = get_arg_real(ep, 1);
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

				result = get_arg_integer(ep, 1);
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

				result = get_arg_string(ep);
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
		scp = sub_context_new();
		sub_var_set_charstar(scp, "Function", "sprintf");
		sub_var_set_long(scp, "Number1", (long)argc);
		sub_var_set_long(scp, "Number2", (long)ai);
		s =
			subst_intl
			(
				scp,
      i18n("$function: too many arguments ($number1 given, only $number2 used)")
			);
		sub_context_delete(scp);
		result = rpt_value_error(ep->pos, s);
		str_free(s);
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
	return result;
}


rpt_func_ty rpt_func_sprintf =
{
	"sprintf",
	1, /* optimizable */
	verify,
	run
};
