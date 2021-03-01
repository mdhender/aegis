/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993 Peter Miller.
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
 * MANIFEST: lexical analysis of command line arguments
 */

#include <stddef.h>
#include <string.h>
#include <ctype.h>

#include <arglex.h>
#include <error.h>
#include <option.h>
#include <str.h>
#include <trace.h>

static arglex_table_ty table[] =
{
	{ "-",			arglex_token_stdio,		},
	{ "-Help",		arglex_token_help,		},
	{ "-VERSion",		arglex_token_version,		},
	{ "-TRace",		arglex_token_trace,		},
};

static	int		argc;
static	char		**argv;
	arglex_value_ty	arglex_value;
	arglex_token_ty	arglex_token;
static	arglex_table_ty	*utable;
static	char		*partial;


/*
 * NAME
 *	arglex_init
 *
 * SYNOPSIS
 *	void arglex_init(int ac, char **av, arglex_table-t *tp);
 *
 * DESCRIPTION
 *	The arglex_init function is used to initialize the
 *	command line processing.
 *
 * ARGUMENTS
 *	ac	- aergument count, from main
 *	av	- argument values, from main
 *	tp	- pointer to table of options
 *
 * CAVEAT
 *	Must be called before the arglex() function.
 */

void
arglex_init(ac, av, tp)
	int		ac;
	char		**av;
	arglex_table_ty	*tp;
{
	option_progname_set(av[0]);
	argc = ac - 1;
	argv = av + 1;
	utable = tp;
}


/*
 * NAME
 *	arglex_compare
 *
 * SYNOPSIS
 *	int arglex_compare(char *formal, char *actual);
 *
 * DESCRIPTION
 *	The arglex_compare function is used to compare
 *	a command line string with a formal spec of the option,
 *	to see if they compare equal.
 *
 *	The actual is case-insensitive.  Uppercase in the formal
 *	means a mandatory character, while lower case means optional.
 *	Any number of consecutive optional characters may be supplied
 *	by actual, but none may be skipped, unless all are skipped to
 *	the next non-lower-case letter.
 *
 *	The underscore (_) is like a lower-case minus,
 *	it matches "", "-" and "_".
 *
 *	The "*" in a pattern matches everything to the end of the line,
 *	anything after the "*" is ignored.  The rest of the line is pointed
 *	to by the "partial" variable as a side-effect (else it will be 0).
 *	This rather ugly feature is to support "-I./dir" type options.
 *
 *	A backslash in a pattern nominates an exact match required,
 *	case must matche excatly here.
 *	This rather ugly feature is to support "-I./dir" type options.
 *
 *	For example: "-project" and "-P' both match "-Project",
 *	as does "-proJ", but "-prj" does not.
 *
 *	For example: "-devDir" and "-d_d' both match "-Development_Directory",
 *	but "-dvlpmnt_drctry" does not.
 *
 *	For example: to match include path specifications, use a pattern
 *	such as "-\\I*", and the partial global variable will have the
 *	path in it on return.
 *
 * ARGUMENTS
 *	formal	- the "pattern" for the option
 *	actual	- what the user supplied
 *
 * RETURNS
 *	int;	zero if no match,
 *		non-zero if they do match.
 */

int
arglex_compare(formal, actual)
	char	*formal;
	char	*actual;
{
	char	fc;
	char	ac;
	int	result;

	trace(("arglex_compare(formal = \"%s\", actual = \"%s\")\n{\n",
		formal, actual));
	for (;;)
	{
		trace_string(formal);
		trace_string(actual);
		ac = *actual++;
		if (isupper(ac))
			ac = tolower(ac);
		fc = *formal++;
		switch (fc)
		{
		case 0:
			result = !ac;
			goto done;
			
		case '_':
			if (ac == '-')
				break;
			/* fall through... */

		case 'a': case 'b': case 'c': case 'd': case 'e':
		case 'f': case 'g': case 'h': case 'i': case 'j':
		case 'k': case 'l': case 'm': case 'n': case 'o':
		case 'p': case 'q': case 'r': case 's': case 't':
		case 'u': case 'v': case 'w': case 'x': case 'y':
		case 'z': 
			/*
			 * optional characters
			 */
			if (ac == fc && arglex_compare(formal, actual))
			{
				result = 1;
				goto done;
			}

			/*
			 * skip forward to next
			 * mandatory character, or after '_'
			 */
			while (islower(*formal))
				++formal;
			if (*formal == '_')
			{
				++formal;
				if (ac == '_' || ac == '-')
					++actual;
			}
			--actual;
			break;

		case '*':
			/*
			 * This is a hack, it should really 
			 * check for a match match the stuff after
			 * the '*', too, a la glob.
			 */
			if (!ac)
			{
				result = 0;
				goto done;
			}
			partial = actual - 1;
			result = 1;
			goto done;

		case '\\':
			if (actual[-1] != *formal++)
			{
				result = 0;
				goto done;
			}
			break;

		case 'A': case 'B': case 'C': case 'D': case 'E':
		case 'F': case 'G': case 'H': case 'I': case 'J':
		case 'K': case 'L': case 'M': case 'N': case 'O':
		case 'P': case 'Q': case 'R': case 'S': case 'T':
		case 'U': case 'V': case 'W': case 'X': case 'Y':
		case 'Z': 
			fc = tolower(fc);
			/* fall through... */

		default:
			/*
			 * mandatory characters
			 */
			if (fc != ac)
			{
				result = 0;
				goto done;
			}
			break;
		}
	}
	done:
	trace(("return %d;\n}\n", result));
	return result;
}


/*
 * NAME
 *	is_a_number
 *
 * SYNOPSIS
 *	int is_a_number(char *s);
 *
 * DESCRIPTION
 *	The is_a_number function is used to determine if the
 *	argument is a number.
 *
 *	The value is placed in arglex_value.alv_number as
 *	a side effect.
 *
 *	Negative and positive signs are accepted.
 *	The C conventions for decimal, octal and hexadecimal are understood.
 *
 *	There may be no white space anywhere in the string,
 *	and the string must end after the last digit.
 *	Trailing garbage will be interpreted to mean it is not a string.
 *
 * ARGUMENTS
 *	s	- string to be tested and evaluated
 *
 * RETURNS
 *	int;	zero if not a number,
 *		non-zero if is a number.
 */

static int is_a_number _((char *));

static int
is_a_number(s)
	char	*s;
{
	long	n;
	int	sign;

	n = 0;
	switch (*s)
	{
	case '-':
		++s;
		sign = -1;
		break;

	case '+':
		++s;
		sign = 1;
		break;

	default:
		sign = 1;
		break;
	}
	switch (*s)
	{
	case '0':
		if ((s[1] == 'x' || s[1] == 'X') && s[2])
		{
			s += 2;
			for (;;)
			{
				switch (*s)
				{
				case '0': case '1': case '2': case '3':
				case '4': case '5': case '6': case '7':
				case '8': case '9':
					n = n * 16 + *s++ - '0';
					continue;

				case 'A': case 'B': case 'C':
				case 'D': case 'E': case 'F':
					n = n * 16 + *s++ - 'A' + 10;
					continue;

				case 'a': case 'b': case 'c':
				case 'd': case 'e': case 'f':
					n = n * 16 + *s++ - 'a' + 10;
					continue;
				}
				break;
			}
		}
		else
		{
			for (;;)
			{
				switch (*s)
				{
				case '0': case '1': case '2': case '3':
				case '4': case '5': case '6': case '7':
					n = n * 8 + *s++ - '0';
					continue;
				}
				break;
			}
		}
		break;

	case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		for (;;)
		{
			switch (*s)
			{
			case '0': case '1': case '2': case '3':
			case '4': case '5': case '6': case '7':
			case '8': case '9':
				n = n * 10 + *s++ - '0';
				continue;
			}
			break;
		}
		break;

	default:
		return 0;
	}
	if (*s)
		return 0;
	arglex_value.alv_number = n * sign;
	return 1;
}


/*
 * NAME
 *	arglex
 *
 * SYNOPSIS
 *	arglex_token_ty arglex(void);
 *
 * DESCRIPTION
 *	The arglex function is used to perfom lexical analysis
 *	on the command line arguments.
 *
 *	Unrecognised options are returned as arglex_token_option
 *	for anything starting with a '-', or
 *	arglex_token_string otherwise.
 *
 * RETURNS
 *	The next token in the token stream.
 *	When the end is reached, arglex_token_eoln is returned forever.
 *
 * CAVEAT
 *	Must call arglex_init befor this function is called.
 */

arglex_token_ty
arglex()
{
	arglex_table_ty	*tp;
	int		j;
	arglex_table_ty	*hit[20];
	int		nhit;
	static char	*pushback;
	char		*arg;

	trace(("arglex()\n{\n"));
	if (pushback)
	{
		/*
		 * the second half of a "-foo=bar" style argument.
		 */
		arg = pushback;
		pushback = 0;
	}
	else
	{
		if (argc <= 0)
		{
			arglex_token = arglex_token_eoln;
			arg = "";
			goto done;
		}
		arg = argv[0];
		argc--;
		argv++;

		/*
		 * See if it looks like a GNU "-foo=bar" option.
		 * Split it at the '=' to make it something the
		 * rest of the code understands.
		 */
		if (arg[0] == '-' && arg[1] != '=')
		{
			char	*eqp;

			eqp = strchr(arg, '=');
			if (eqp)
			{
				pushback = eqp + 1;
				*eqp = 0;
			}
		}

		/*
		 * Turn the GNU-style leading "--"
		 * into "-" if necessary.
		 */
		if
		(
			arg[0] == '-'
		&&
			arg[1] == '-'
		&&
			arg[2]
		&&
			!is_a_number(arg + 1)
		)
			++arg;
	}

	/*
	 * see if it is a number
	 */
	if (is_a_number(arg))
	{
		arglex_token = arglex_token_number;
		goto done;
	}

	/*
	 * scan the tables to see what it matches
	 */
	nhit = 0;
	partial = 0;
	for (tp = table; tp < ENDOF(table); tp++)
	{
		if (arglex_compare(tp->t_name, arg))
			hit[nhit++] = tp;
	}
	if (utable)
	{
		for (tp = utable; tp->t_name; tp++)
		{
			if (arglex_compare(tp->t_name, arg))
				hit[nhit++] = tp;
		}
	}

	/*
	 * deal with unknown or ambiguous options
	 */
	switch (nhit)
	{
	case 0:
		/*
		 * not found in the tables
		 */
		if (*arg == '-')
			arglex_token = arglex_token_option;
		else
			arglex_token = arglex_token_string;
		break;

	case 1:
		arglex_token = hit[0]->t_token;
		if (partial)
			arg = partial;
		else
			arg = hit[0]->t_name;
		break;

	default:
		{
			string_ty	*s1;
			string_ty	*s2;

			s1 = str_from_c(hit[0]->t_name);
			for (j = 1; j < nhit; ++j)
			{
				s2 = str_format("%S, %s", s1, hit[j]->t_name);
				str_free(s1);
				s2 = s2;
			}
			fatal
			(
				"option \"%s\" abmiguous (%s)",
				arg,
				s1->str_text
			);
		}
	}

	/*
	 * here for all exits
	 */
	done:
	arglex_value.alv_string = arg;
	trace(("return %d; /* %s */\n", arglex_token, arglex_value.alv_string));
	trace((/*{*/"}\n"));
	return arglex_token;
}
