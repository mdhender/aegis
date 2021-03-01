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
 * MANIFEST: symbol table manipulation
 */

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <main.h>
#include <id.h>
#include <word.h>
#include <mem.h>
#include <error.h>


#define HEADER \
	string_ty	*id_name; \
	str_hash_ty	id_hash; \
	id		*id_next; \
	id_class_ty	id_class;

typedef struct id id;
struct id
{
	HEADER
};

typedef struct id_int id_int;
struct id_int
{
	HEADER
	int		id_value;
};

typedef struct id_void id_void;
struct id_void
{
	HEADER
	void		*id_value;
};

static	id		**hash_table;
static	str_hash_ty	hash_modulus;
static	str_hash_ty	hash_cutover;
static	str_hash_ty	hash_cutover_mask;
static	str_hash_ty	hash_cutover_split_mask;
static	str_hash_ty	hash_split;
static	str_hash_ty	hash_load;


/*
 * NAME
 *	id_initialize - start up symbol table
 *
 * SYNOPSIS
 *	void id_initialize(void);
 *
 * DESCRIPTION
 *	The id_initialize function is used to create the hash table.
 *
 * CAVEAT
 *	Assumes the str_initialize function has been called already.
 */

void
id_initialize()
{
	str_hash_ty	j;

	hash_modulus = 1<<8; /* MUST be a power of 2 */
	hash_cutover = hash_modulus;
	hash_split = hash_modulus - hash_cutover;
	hash_cutover_mask = hash_cutover - 1;
	hash_cutover_split_mask = (hash_cutover * 2) - 1;
	hash_load = 0;
	hash_table = (id **)mem_alloc(hash_modulus * sizeof(id *));
	for (j = 0; j < hash_modulus; ++j)
		hash_table[j] = 0;
}


/*
 * NAME
 *	split - reduce symbol table load
 *
 * SYNOPSIS
 *	void split(void);
 *
 * DESCRIPTION
 *	The split function is used to split symbols in the bucket indicated by
 *	the split point.  The symbols are split between that bucket and the one
 *	after the current end of the table.
 *
 * CAVEAT
 *	It is only sensable to do this when the symbol table load exceeds some
 *	reasonable threshold.  A threshold of 80% is suggested.
 */

static void split _((void));

static void
split()
{
	id		*p;
	id		**ipp;
	id		*p2;
	str_hash_ty	index;

	/*
	 * get the list to be split across buckets 
	 */
	p = hash_table[hash_split];
	hash_table[hash_split] = 0;

	/*
	 * increase the modulus by one
	 */
	hash_modulus++;
	mem_change_size((char **)&hash_table, hash_modulus * sizeof(id *));
	hash_table[hash_modulus - 1] = 0;
	hash_split = hash_modulus - hash_cutover;
	if (hash_split >= hash_cutover)
	{
		hash_cutover = hash_modulus;
		hash_split = 0;
		hash_cutover_mask = hash_cutover - 1;
		hash_cutover_split_mask = (hash_cutover * 2) - 1;
	}

	/*
	 * now redistribute the list elements
	 *
	 * It is important to preserve the order of the links because
	 * they can be push-down stacks, and to simply add them to the
	 * head of the list will reverse the order of the stack!
	 */
	while (p)
	{
		p2 = p;
		p = p2->id_next;
		p2->id_next = 0;

		index = p2->id_hash & hash_cutover_mask;
		if (index < hash_split)
			index = p2->id_hash & hash_cutover_split_mask;
		for (ipp = &hash_table[index]; *ipp; ipp = &(*ipp)->id_next)
			;
		*ipp = p2;
	}
}


/*
 * NAME
 *	copy - copy a symbol value
 *
 * SYNOPSIS
 *	void copy(id *p, long *valuep);
 *
 * DESCRIPTION
 *	The copy function is used to copy the passed value of a symbol into the
 *	storage of that symbol.
 */

static void copy _((id *, long *));

static void
copy(p, valuep)
	id	*p;
	long	*valuep;
{
	switch (p->id_class)
	{
	case ID_CLASS_KEYWORD:
		{
			id_int	*ip;

			ip = (id_int *)p;
			*valuep = ip->id_value;
		}
		break;

	case ID_CLASS_ENUMEL:
	case ID_CLASS_FIELD:
	case ID_CLASS_TYPE:
		{
			id_void	*ip;

			ip = (id_void *)p;
			*(void **)valuep = ip->id_value;
		}
		break;
	}
}


/*
 * NAME
 *	id_search - search for a variable
 *
 * SYNOPSIS
 *	int id_search(string_ty *name, id_class_ty class, long *value);
 *
 * DESCRIPTION
 *	Id_search is used to reference a variable.
 *
 * RETURNS
 *	If the variable has been defined, the function returns a non-zero value
 *	and the value is returned through the 'value' pointer.
 *	If the variable has not been defined, it returns zero,
 *	and 'value' is unaltered.
 *
 * CAVEAT
 *	The value returned from this function, when returned, is allocated
 *	in dynamic memory (it is a copy of the value remembered by this module).
 *	It is the responsibility of the caller to free it when finished with,
 *	by a wl_free() call.
 */

/*VARARGS2*/
int
id_search(name, class, valuep)
	string_ty	*name;
	id_class_ty	class;
	long		*valuep;
{
	str_hash_ty	myhash;
	str_hash_ty	index;
	id		*p;

	myhash = name->str_hash + (int)class;
	index = myhash & hash_cutover_mask;
	if (index < hash_split)
		index = myhash & hash_cutover_split_mask;
	for (p = hash_table[index]; p; p = p->id_next)
	{
		if (p->id_class == class && str_equal(name, p->id_name))
		{
			copy(p, valuep);
			return 1;
		}
	}
	return 0;
}


/*
 * NAME
 *	assign - set value of symbol
 *
 * SYNOPSIS
 *	void assign(id *p, long value);
 *
 * DESCRIPTION
 *	The assign function is used to change the value of a symbol.
 *
 * CAVEAT
 *	The value is not released first, so use stomp if necessary.
 */

static void assign _((id *, long));

static void
assign(p, value)
	id	*p;
	long	value;
{
	switch (p->id_class)
	{
	case ID_CLASS_KEYWORD:
		{
			id_int	*ip;

			ip = (id_int *)p;
			ip->id_value = value;
		}
		break;

	case ID_CLASS_TYPE:
	case ID_CLASS_FIELD:
	case ID_CLASS_ENUMEL:
		{
			id_void	*ip;

			ip = (id_void *)p;
			ip->id_value = (void *)value;
		}
		break;
	}
}


/*
 * NAME
 *	size_by_class - determine symbol size
 *
 * SYNOPSIS
 *	size_t size_by_class(id_class_ty class);
 *
 * DESCRIPTION
 *	The size_by_class function is used to determine the correct size to
 *	allocate for symbol storage.
 *
 * RETURNS
 *	size_t: the correct size to malloc.
 *
 * CAVEAT
 *	Never malloc(sizeof(id)) as this will be too small.
 */

static size_t size_by_class _((id_class_ty));

static size_t
size_by_class(class)
	id_class_ty class;
{
	switch ((int)class)
	{
	default:
		assert(0);

	case ID_CLASS_KEYWORD:
		return sizeof(id_int);

	case ID_CLASS_TYPE:
	case ID_CLASS_FIELD:
	case ID_CLASS_ENUMEL:
		return sizeof(id_void);
	}
}


/*
 * NAME
 *	id_assign - assign a variable
 *
 * SYNOPSIS
 *	void id_assign(string_ty *name, id_class_ty class, long value);
 *
 * DESCRIPTION
 *	Id_assign is used to assign a value to a given variable.
 *
 * CAVEAT
 *	The name and value are copied by id_assign, so the user may
 *	modify or free them at a later date without affecting the
 *	variable.
 */

/*VARARGS2*/
void
id_assign(name, class, value)
	string_ty	*name;
	id_class_ty	class;
	long		value;
{
	str_hash_ty	myhash;
	str_hash_ty	index;
	id		*p;

	myhash = name->str_hash + (int)class;
	index = myhash & hash_cutover_mask;
	if (index < hash_split)
		index = myhash & hash_cutover_split_mask;

	for (p = hash_table[index]; p; p = p->id_next)
	{
		if (p->id_class == class && str_equal(name, p->id_name))
		{
			assign(p, value);
			return;
		}
	}

	p = (id *)mem_alloc_clear(size_by_class(class));
	p->id_name = str_copy(name);
	p->id_next = hash_table[index];
	p->id_class = class;
	p->id_hash = myhash;
	hash_table[index] = p;
	assign(p, value);

	hash_load++;
	while (hash_load * 10 >= hash_modulus * 8)
		split();
}


/*
 * NAME
 *	id_dump - dump id table
 *
 * SYNOPSIS
 *	void id_dump(char *title, int mask);
 *
 * DESCRIPTION
 *	The id_dump function is used to dump the contents of the id table.
 *	The title will be used to indicate why the table was dumped.  The mask
 *	may be used to selectively dump the table, 0 means everything, bits
 *	correspond directly with ID_CLASS defines.
 *
 * CAVEAT
 *	This function is only available when symbol DEBUG is defined.
 */

#ifdef DEBUG

void
id_dump(s, mask)
	char	*s;
	int	mask;
{
	int	j;
	id	*p;

	if (!mask)
		mask = ~0;
	error("id table %s = {"/*}*/, s);
	for (j = 0; j < hash_modulus; ++j)
	{
		for (p = hash_table[j]; p; p = p->id_next)
		{
			if (mask & (1<< (int)p->id_class))
			switch ((int)p->id_class)
			{
			default:
				error
				(
					"name = \"%s\", class = %d",
					p->id_name->str_text,
					p->id_class
				);
				break;

			case ID_CLASS_KEYWORD:
				{
					id_int	*ip;
		
					ip = (id_int*)p;
					error
					(
					"name = \"%s\", class = %d, value = %d",
						p->id_name->str_text,
						p->id_class,
						ip->id_value
					);
				}
				break;

			case ID_CLASS_TYPE:
			case ID_CLASS_FIELD:
				{
					id_void	*ip;
		
					ip = (id_void *)p;
					error
					(
				      "name = \"%s\", class = %d, value = %08X",
						p->id_name->str_text,
						p->id_class,
						ip->id_value
					);
				}
				break;
			}
		}
	}
	error(/*{*/"}");
}

#endif
