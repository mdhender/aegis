/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1995 Peter Miller;
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
 * MANIFEST: functions to provide consistent treatment of -Help options
 */

#include <ctype.h>
#include <stdio.h>
#include <ac/string.h>
#include <ac/stdlib.h>
#include <ac/unistd.h>

#include <arglex2.h>
#include <error.h>
#include <gonzo.h>
#include <help.h>
#include <mem.h>
#include <option.h>
#include <pager.h>
#include <trace.h>
#include <undo.h>
#include <version_stmp.h>


#define PAIR(a, b) ((a) * 256 + (b))


static	FILE	*fp;

static char *cr[] =
{
	"\\*(n) version \\*(v)",
	".br",
	"Copyright (C) \\*(Y) Peter Miller;",
	"All rights reserved.",
	"",
	"The \\*(n) program comes with ABSOLUTELY NO WARRANTY;",
	"for details use the '\\*(n) -VERSion Warranty' command.",
	"The \\*(n) program is free software, and you are welcome to",
	"redistribute it under certain conditions;",
	"for details use the '\\*(n) -VERSion Redistribution' command.",
};

static char *au[] =
{
	".nf",
	"Peter Miller       UUCP: uunet!munnari!agso.gov.au!pmiller",
	"/\\e/\\e*          Internet: pmiller@agso.gov.au",
	".fi",
};

char *so_o__rules[] =
{
#include <../man1/o__rules.h>
};

char *so_o_anticip[] =
{
#include <../man1/o_anticip.h>
};

char *so_o_auto[] =
{
#include <../man1/o_auto.h>
};

char *so_o_baselin[] =
{
#include <../man1/o_baselin.h>
};

char *so_o_bld_lk[] =
{
#include <../man1/o_bld_lk.h>
};

char *so_o_change[] =
{
#include <../man1/o_change.h>
};

char *so_o_devdir[] =
{
#include <../man1/o_devdir.h>
};

char *so_o_dir[] =
{
#include <../man1/o_dir.h>
};

char *so_o_edit[] =
{
#include <../man1/o_edit.h>
};

char *so_o_file[] =
{
#include <../man1/o_file.h>
};

char *so_o_help[] =
{
#include <../man1/o_help.h>
};

char *so_o_indep[] =
{
#include <../man1/o_indep.h>
};

char *so_o_interact[] =
{
#include <../man1/o_interact.h>
};

char *so_o_keep[] =
{
#include <../man1/o_keep.h>
};

char *so_o_lib[] =
{
#include <../man1/o_lib.h>
};

char *so_o_list[] =
{
#include <../man1/o_list.h>
};

char *so_o_major[] =
{
#include <../man1/o_major.h>
};

char *so_o_manual[] =
{
#include <../man1/o_manual.h>
};

char *so_o_min[] =
{
#include <../man1/o_min.h>
};

char *so_o_minor[] =
{
#include <../man1/o_minor.h>
};

char *so_o_nolog[] =
{
#include <../man1/o_nolog.h>
};

char *so_o_overw[] =
{
#include <../man1/o_overw.h>
};

char *so_o_page[] =
{
#include <../man1/o_page.h>
};

char *so_o_project[] =
{
#include <../man1/o_project.h>
};

char *so_o_regress[] =
{
#include <../man1/o_regress.h>
};

char *so_o_symlnk[] =
{
#include <../man1/o_symlnk.h>
};

char *so_o_terse[] =
{
#include <../man1/o_terse.h>
};

char *so_o_unforma[] =
{
#include <../man1/o_unforma.h>
};

char *so_o_user[] =
{
#include <../man1/o_user.h>
};

char *so_o_verbose[] =
{
#include <../man1/o_verbose.h>
};

char *so_z_cr[] =
{
	".SH COPYRIGHT",
	".so cr",
	".SH AUTHOR",
	".so au",
};

char *so_z_exit[] =
{
#include <../man1/z_exit.h>
};

char *so_z_intuit[] =
{
#include <../man1/z_intuit.h>
};


typedef struct so_list_ty so_list_ty;
struct so_list_ty
{
	char	*name;
	char	**text;
	int	length;
};

static so_list_ty so_list[] =
{
	{ "o__rules.so",	so_o__rules,	SIZEOF(so_o__rules)	},
	{ "o_anticip.so",	so_o_anticip,	SIZEOF(so_o_anticip)	},
	{ "o_auto.so",		so_o_auto,	SIZEOF(so_o_auto)	},
	{ "o_baselin.so",	so_o_baselin,	SIZEOF(so_o_baselin)	},
	{ "o_bld_lk.so",	so_o_bld_lk,	SIZEOF(so_o_bld_lk)	},
	{ "o_change.so",	so_o_change,	SIZEOF(so_o_change)	},
	{ "o_devdir.so",	so_o_devdir,	SIZEOF(so_o_devdir)	},
	{ "o_dir.so",		so_o_dir,	SIZEOF(so_o_dir)	},
	{ "o_edit.so",		so_o_edit,	SIZEOF(so_o_edit)	},
	{ "o_file.so",		so_o_file,	SIZEOF(so_o_file)	},
	{ "o_help.so",		so_o_help,	SIZEOF(so_o_help)	},
	{ "o_indep.so",		so_o_indep,	SIZEOF(so_o_indep)	},
	{ "o_interact.so",	so_o_interact,	SIZEOF(so_o_interact)	},
	{ "o_keep.so",		so_o_keep,	SIZEOF(so_o_keep)	},
	{ "o_lib.so",		so_o_lib,	SIZEOF(so_o_lib)	},
	{ "o_list.so",		so_o_list,	SIZEOF(so_o_list)	},
	{ "o_major.so",		so_o_major,	SIZEOF(so_o_major)	},
	{ "o_manual.so",	so_o_manual,	SIZEOF(so_o_manual)	},
	{ "o_min.so",		so_o_min,	SIZEOF(so_o_min)	},
	{ "o_minor.so",		so_o_minor,	SIZEOF(so_o_minor)	},
	{ "o_nolog.so",		so_o_nolog,	SIZEOF(so_o_nolog)	},
	{ "o_overw.so",		so_o_overw,	SIZEOF(so_o_overw)	},
	{ "o_page.so",		so_o_page,	SIZEOF(so_o_page)	},
	{ "o_project.so",	so_o_project,	SIZEOF(so_o_project)	},
	{ "o_regress.so",	so_o_regress,	SIZEOF(so_o_regress)	},
	{ "o_symlnk.so",	so_o_symlnk,	SIZEOF(so_o_symlnk)	},
	{ "o_terse.so",		so_o_terse,	SIZEOF(so_o_terse)	},
	{ "o_unforma.so",	so_o_unforma,	SIZEOF(so_o_unforma)	},
	{ "o_user.so",		so_o_user,	SIZEOF(so_o_user)	},
	{ "o_verbose.so",	so_o_verbose,	SIZEOF(so_o_verbose)	},
	{ "z_cr.so",		so_z_cr,	SIZEOF(so_z_cr)		},
	{ "z_exit.so",		so_z_exit,	SIZEOF(so_z_exit)	},
	{ "z_intuit.so",	so_z_intuit,	SIZEOF(so_z_intuit)	},
	{ "z_name.so",		0,		0			},
	{ "../doc/version.so",	0,		0			},
	{ "cr",			cr,		SIZEOF(cr),		},
	{ "au",			au,		SIZEOF(au),		},
};


static	int	ocol;
static	int	icol;
static	int	fill;	/* true if currently filling */
static	int	in;	/* current indent */
static	int	in_base;	/* current paragraph indent */
static	int	ll;	/* line length */
static	long	roff_line;
static	char	*roff_file;
static	int	TP_line;


static void emit _((int));

static void
emit(c)
	int	c;
{
	switch (c)
	{
	case ' ':
		icol++;
		break;

	case '\t':
		icol = ((icol / 8) + 1) * 8;
		break;
	
	case '\n':
		fputc('\n', fp);
		fflush(fp);
		icol = 0;
		ocol = 0;
		break;

	default:
		if (!isprint(c))
			break;
		while (((ocol / 8) + 1) * 8 <= icol && ocol + 1 < icol)
		{
			fputc('\t', fp);
			ocol = ((ocol / 8) + 1) * 8;
		}
		while (ocol < icol)
		{
			fputc(' ', fp);
			++ocol;
		}
		fputc(c, fp);
		++icol;
		++ocol;
		break;
	}
	if (ferror(fp))
		pager_error(fp);
}


static void emit_word _((char *, long));

static void
emit_word(buf, len)
	char	*buf;
	long	len;
{
	if (len <= 0)
		return;

	/*
	 * if this line is not yet indented, indent it
	 */
	if (!ocol && !icol)
		icol = in;
	
	/*
	 * if there is already something on this line 
	 * and we are in "fill" mode
	 * and this word would cause it to overflow
	 * then wrap the line
	 */
	if (ocol && fill && icol + len >= ll)
	{
		emit('\n');
		icol = in;
	}
	if (ocol)
		emit(' ');
	while (len-- > 0)
		emit(*buf++);
}


static void br _((void));

static void
br()
{
	if (ocol)
		emit('\n');
}


static void sp _((void));

static void
sp()
{
	br();
	emit('\n');
}


static void interpret_line_of_words _((char *));

static void
interpret_line_of_words(line)
	char	*line;
{
	/*
	 * if not filling,
	 * pump the line out literrally.
	 */
	if (!fill)
	{
		if (!ocol && !icol)
			icol = in;
		while (*line)
			emit(*line++);
		emit('\n');
		return;
	}

	/*
	 * in fill mode, a blank line means
	 * finish the paragraph and emit a blank line
	 */
	if (!*line)
	{
		sp();
		return;
	}

	/*
	 * break the line into space-separated words
	 * and emit each individually
	 */
	while (*line)
	{
		char	*start;

		while (isspace(*line))
			++line;
		if (!*line)
			break;
		start = line;
		while (*line && !isspace(*line))
			++line;
		emit_word(start, line - start);

		/*
		 * extra space at end of sentences
		 */
		if
		(
			(line[-1] == '.' || line[-1] == '?')
		&&
			(
				!line[0]
			||
				(
					line[0] == ' '
				&&
					(!line[1] || line[1] == ' ')
				)
			)
		)
			emit(' ');
	}
}


static void roff_error _((char *, ...));

static void
roff_error(s sva_last)
	char		*s;
	sva_last_decl
{
	char		buffer[1000];
	va_list		ap;

	sva_init(ap, s);
	vsprintf(buffer, s, ap);
	va_end(ap);

#if 0
	br();
	if (roff_file)
		emit_word(roff_file, strlen(roff_file));
	if (roff_line)
	{
		char line[20];
		sprintf(line, "%ld", roff_line);
		emit_word(line, strlen(line));
	}
	interpret_line_of_words(buffer);
	br();
#else
	fatal
	(
		"%s: %ld: %s",
		(roff_file ? roff_file : "(noname)"),
		roff_line,
		buffer
	);
#endif
}


static void get_name _((char **, char *));

static void
get_name(lp, name)
	char	**lp;
	char	*name;
{
	char	*line;

	line = *lp;
	if (*line == '('/*)*/)
	{
		++line;
		if (*line)
		{
			name[0] = *line++;
			if (*line)
			{
				name[1] = *line++;
				name[2] = 0;
			}
			else
				name[1] = 0;
		}
		else
			name[0] = 0;
	}
	else if (*line)
	{
		name[0] = *line++;
		name[1] = 0;
	}
	else
		name[0] = 0;
	*lp = line;
}


typedef struct string_reg_ty string_reg_ty;
struct string_reg_ty
{
	char	*name;
	char	*value;
};


static	long		string_reg_count;
static	string_reg_ty	*string_reg;



static char *string_find _((char *));

static char *
string_find(name)
	char	*name;
{
	long	j;

	for (j = 0; j < string_reg_count; ++j)
	{
		string_reg_ty	*srp;

		srp = &string_reg[j];
		if (!strcmp(name, srp->name))
			return srp->value;
	}
	return 0;
}


static char *numreg_find _((char *));

static char *
numreg_find(name)
	char	*name;
{
	return 0;
}


static void roff_prepro _((char *, char *));

static void
roff_prepro(buffer, line)
	char	*buffer;
	char	*line;
{
	char	*bp;
	char	*value;
	char	name[4];

	bp = buffer;
	while (*line)
	{
		int c = *line++;
		if (c != '\\')
		{
			*bp++ = c;
			continue;
		}
		c = *line++;
		if (!c)
		{
			roff_error("can't do escaped end-of-line");
			break;
		}
		switch (c)
		{
		default:
			roff_error("unknown \\%c inline directive", c);
			break;

		case '%':
			/* word break info */
			break;

		case '*':
			/* inline string */
			get_name(&line, name);
			value = string_find(name);
			if (value)
			{
				while (*value)
					*bp++ = *value++;
			}
			break;

		case 'n':
			/* inline number register */
			get_name(&line, name);
			value = numreg_find(name);
			if (value)
			{
				while (*value)
					*bp++ = *value++;
			}
			break;

		case 'e':
		case '\\':
			*bp++ = '\\';
			break;

		case '-':
			*bp++ = '-';
			break;

		case 'f':
			/* ignore font directives */
			get_name(&line, name);
			break;

		case '&':
		case '|':
			/* ignore weird space directives */
			break;
		}
	}
	*bp = 0;
}


static void interpret_text _((char *));

static void
interpret_text(line)
	char	*line;
{
	char	buffer[1000];

	roff_prepro(buffer, line);
	interpret_line_of_words(buffer);
	if (TP_line)
	{
		if (icol >= 15)
			br();
		else
			icol = 15;
		TP_line = 0;
		in = in_base + 8;
	}
}


static void roff_sub _((char *, int, char **));

static void
roff_sub(buffer, argc, argv)
	char	*buffer;
	int	argc;
	char	**argv;
{
	int	j;
	char	*bp;
	long	len;

	bp = buffer;
	for (j = 0; j < argc; ++j)
	{
		len = strlen(argv[j]);
		if (j)
			*bp++ = ' ';
		memcpy(bp, argv[j], len);
		bp += len;
	}
	*bp = 0;
}


static void interpret_text_args _((int, char **));

static void
interpret_text_args(argc, argv)
	int	argc;
	char	**argv;
{
	char	buffer[1000];

	roff_sub(buffer, argc, argv);
	interpret_text(buffer);
}


static void concat_text_args _((int, char **));

static void
concat_text_args(argc, argv)
	int	argc;
	char	**argv;
{
	int	j;
	char	*bp;
	long	len;
	char	buffer[1000];

	bp = buffer;
	for (j = 0; j < argc; ++j)
	{
		len = strlen(argv[j]);
		if ((bp - buffer) + len + 1 >= sizeof(buffer))
			break;
		memcpy(bp, argv[j], len);
		bp += len;
	}
	*bp = 0;
	interpret_text(buffer);
}


static void interpret _((char **, int)); /* forward */


static void so _((int, char **));

static void
so(argc, argv)
	int	argc;
	char	**argv;
{
	so_list_ty	*sop;

	if (argc != 1)
	{
		roff_error(".so requires one argument");
		return;
	}
	for (sop = so_list; sop < ENDOF(so_list); ++sop)
	{
		if (!strcmp(sop->name, argv[0]))
		{
			interpret(sop->text, sop->length);
			return;
		}
	}
	roff_error("\".so %s\" not known", argv[0]);
}


static void lf _((int, char **));

static void
lf(argc, argv)
	int	argc;
	char	**argv;
{
	if (roff_file)
		mem_free(roff_file);
	if (argc >= 1)
		roff_line = atol(argv[0]) - 1;
	else
		roff_line = 0;
	if (argc >= 2)
		roff_file = mem_copy_string(argv[1]);
	else
		roff_file = 0;
}


static void ds_guts _((char *, char *));

static void
ds_guts(name, value)
	char		*name;
	char		*value;
{
	long		j;
	string_reg_ty	*srp;

	for (j = 0; j < string_reg_count; ++j)
	{
		srp = &string_reg[j];
		if (!strcmp(name, srp->name))
		{
			mem_free(srp->value);
			srp->value = mem_copy_string(value);
			return;
		}
	}

	if (string_reg_count)
	{
		string_reg =
			mem_change_size
			(
				string_reg,
				(string_reg_count + 1) * sizeof(string_reg_ty)
			);
	}
	else
		string_reg = mem_alloc(sizeof(string_reg_ty));
	srp = &string_reg[string_reg_count++];
	srp->name = mem_copy_string(name);
	srp->value = mem_copy_string(value);
}


static void ds _((int, char **));

static void
ds(argc, argv)
	int	argc;
	char	**argv;
{
	char	buf1[1000];
	char	buf2[1000];

	if (!argc)
		return;
	roff_sub(buf1, argc - 1, argv + 1);
	roff_prepro(buf2, buf1);
	ds_guts(argv[0], buf2);
}


static void dot_in _((int, char**));

static void
dot_in(argc, argv)
	int	argc;
	char	**argv;
{
	if (argc < 1)
		return;
	switch (argv[0][0])
	{
	case '-':
		in -= atoi(argv[0] + 1);
		break;

	case '+':
		in += atoi(argv[0] + 1);
		break;

	default:
		in = atoi(argv[0] + 1);
		break;
	}
	if (in < 0)
		in = 0;
}


static void interpret _((char **, int)); /* forward */


static void interpret_control _((char *));

static void
interpret_control(line)
	char	*line;
{
	int	c1, c2;
	int	argc;
	char	*argv[20];
	char	temp[1000];
	char	*cp;

	/*
	 * find the directive name
	 */
	line++;
	while (isspace(*line))
		++line;
	if (*line)
		c1 = *line++;
	else
		c1 = ' ';
	if (*line)
		c2 = *line++;
	else
		c2 = ' ';

	/*
	 * break the line into space-separated arguments
	 */
	argc = 0;
	cp = temp;
	while (argc < SIZEOF(argv))
	{
		int quoting;

		while (isspace(*line))
			++line;
		if (!*line)
			break;
		argv[argc++] = cp;
		quoting = 0;
		while (*line)
		{
			if (*line == '"')
			{
				quoting = !quoting;
				++line;
				continue;
			}
			if (!quoting && isspace(*line))
				break;
			*cp++ = *line++;
		}
		*cp++ = 0;
		if (!*line)
			break;
	}

	/*
	 * now do something with it
	 */
	switch (PAIR(c1, c2))
	{
	case PAIR('n', 'e'):
		/* ignore the space needed directive */
		break;

	case PAIR('i', 'n'):
		dot_in(argc, argv);
		break;

	case PAIR('I', ' '):
	case PAIR('I', 'R'):
	case PAIR('I', 'B'):
	case PAIR('R', ' '):
	case PAIR('R', 'I'):
	case PAIR('R', 'B'):
	case PAIR('B', ' '):
	case PAIR('B', 'I'):
	case PAIR('B', 'R'):
		concat_text_args(argc, argv);
		break;

	case PAIR('n', 'f'):
		br();
		fill = 0;
		break;

	case PAIR('f', 'i'):
		br();
		fill = 1;
		break;

	case PAIR('t', 'a'):
		/* ignore tab directive */
		break;

	case PAIR('b', 'r'):
		br();
		break;

	case PAIR('s', 'p'):
		sp();
		break;

	case PAIR('I', 'P'):
		in = in_base;
		sp();
		emit(' ');
		emit(' ');
		break;

	case PAIR('P', 'P'):
		in = in_base;
		sp();
		break;

	case PAIR('T', 'H'):
		break;

	case PAIR('T', 'P'):
		in = in_base;
		sp();
		TP_line = 1;
		break;

	case PAIR('S', 'H'):
		in = 0;
		sp();
		interpret_text_args(argc, argv);
		br();
		in_base = 8;
		in = 8;
		break;

	case PAIR('s', 'o'):
		so(argc, argv);
		break;

	case PAIR('l', 'f'):
		lf(argc, argv);
		break;

	case PAIR('R', 'S'):
		in_base = 16;
		in = 16;
		br();
		break;

	case PAIR('R', 'E'):
		in_base = 8;
		in = 8;
		br();
		break;

	case PAIR('d', 's'):
		ds(argc, argv);
		break;

	case PAIR('r', /*(*/')'):
		cp = string_find(/*(*/"R)");
		if (!cp)
			cp = "";
		if (strcmp(cp, "no") != 0)
		{
			static char *macro[] =
			{
				".PP",
				"See also",
				".IR \\*(n) (1)",
				"for options common to all \\*(n) commands.",
			};

			interpret(macro, SIZEOF(macro));
		}
		break;

	default:
		roff_error("formatting directive \".%c%c\" unknown", c1, c2);
		break;
	}
}


static void interpret _((char **, int));

static void
interpret(text, text_len)
	char	**text;
	int	text_len;
{
	int	j;
	long	hold_line;
	char	*hold_file;

	/*
	 * save position
	 */
	trace(("interpret()\n{\n"/*}*/));
	hold_line = roff_line;
	hold_file = roff_file ? mem_copy_string(roff_file) : (char *)0;

	/*
	 * interpret the text
	 */
	for (j = 0; j < text_len; ++j)
	{
		char *s;

		s = text[j];
		if (*s == '.' || *s == '\'')
			interpret_control(s);
		else
			interpret_text(s);
		++roff_line;
		if (ferror(fp))
			pager_error(fp);
	}

	/*
	 * restore position
	 */
	if (roff_file)
		mem_free(roff_file);
	roff_line = hold_line;
	roff_file = hold_file;
	trace((/*{*/"}\n"));
}


void
help(text, text_len, usage)
	char	**text;
	int	text_len;
	void	(*usage)_((void));
{
	/*
	 * collect the rest of thge command line,
	 * if necessary
	 */
	trace(("help(text = %08lX, text_len = %d, usage = %08lX)\n{\n"/*}*/,
		text, text_len, usage));
	if (usage)
	{
		arglex();
		while (arglex_token != arglex_token_eoln)
			generic_argument(usage);
	}

	/*
	 * paginate output if appropriate
	 */
	fp = pager_open();

	/*
	 * initialize the state of the interpreter
	 */
	ds_guts(/*(*/"n)", option_progname_get());
	ds_guts(/*(*/"v)", version_stamp());
	ds_guts(/*(*/"Y)", copyright_years());
	ll = option_page_width_get() - 1;
	if (ll < 40)
		ll = 40;
	in = 0;
	in_base = 0;
	fill = 1;
	ocol = 0;
	icol = 0;
	lf(0, 0);
	TP_line = 0;

	/*
	 * do what they asked
	 */
	interpret(text, text_len);
	br();

	/*
	 * close the paginator
	 */
	pager_close(fp);
	trace((/*{*/"}\n"));
}


void
generic_argument(usage)
	void	(*usage)_((void));
{
	trace(("generic_argument()\n{\n"/*}*/));
	switch (arglex_token)
	{
	default:
		bad_argument(usage);
		/* NOTREACHED */

	case arglex_token_library:
		if (arglex() != arglex_token_string)
			usage();
		gonzo_library_append(arglex_value.alv_string);
		arglex();
		break;

	case arglex_token_page_length:
		if (arglex() != arglex_token_number)
			usage();
		option_page_length_set(arglex_value.alv_number);
		arglex();
		break;

	case arglex_token_page_width:
		if (arglex() != arglex_token_number)
			usage();
		option_page_width_set(arglex_value.alv_number);
		arglex();
		break;

	case arglex_token_tab_width:
		if (arglex() != arglex_token_number)
			usage();
		option_tab_width_set(arglex_value.alv_number);
		arglex();
		break;

	case arglex_token_terse:
		option_terse_set();
		arglex();
		break;

	case arglex_token_trace:
		if (arglex() != arglex_token_string)
			usage();
		for (;;)
		{
			trace_enable(arglex_value.alv_string);
			if (arglex() != arglex_token_string)
				break;
		}
#ifndef DEBUG
		error
		(
"Warning: the -TRace option is only effective when the %s program \
is compiled using the DEBUG define in the conf/main.h include file.",
			option_progname_get()
		);
#endif
		break;

	case arglex_token_unformatted:
		option_unformatted_set();
		arglex();
		break;

	case arglex_token_verbose:
		option_verbose_set();
		arglex();
		break;
	}
	trace((/*{*/"}\n"));
}


void
bad_argument(usage)
	void	(*usage)_((void));
{
	trace(("bad_argument()\n{\n"/*}*/));
	switch (arglex_token)
	{
	case arglex_token_string:
		error("misplaced file name (\"%s\")", arglex_value.alv_string);
		break;

	case arglex_token_number:
		error("misplaced number (%s)", arglex_value.alv_string);
		break;

	case arglex_token_option:
		error("unknown \"%s\" option", arglex_value.alv_string);
		break;

	case arglex_token_eoln:
		error("command line too short");
		break;

	default:
		error("misplaced \"%s\" option", arglex_value.alv_string);
		break;
	}
	usage();
	trace((/*{*/"}\n"));
	quit(1);
	/* NOTREACHED */
}
