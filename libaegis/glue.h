/*
 *	aegis - project change supervisor
 *	Copyright (C) 1993, 1994, 1999 Peter Miller;
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
 * MANIFEST: interface definition for libaegis/glue.c
 */

#ifndef AEGIS_GLUE_H
#define AEGIS_GLUE_H

#include <ac/stdio.h>

#include <main.h>
#include <config.h>

struct stat;
struct utimbuf;
struct flock;

int copyfile _((char *src, char *dst));
int catfile _((char *path));
int read_whole_dir _((char *path, char **data, long *datalen));
int file_compare _((char *, char *));
int file_fingerprint _((char *path, char *buf, int max));
int rmdir_bg _((char *path));
int rmdir_tree _((char *path));

int glue_access _((char *path, int mode));
int glue_catfile _((char *path));
int glue_chmod _((char *path, int mode));
int glue_chown _((char *path, int uid, int gid));
int glue_close _((int fd));
int glue_creat _((char *path, int mode));
int glue_copyfile _((char *src, char *dst));
int glue_fclose _((FILE *));
int glue_file_compare _((char *, char *));
int glue_file_fingerprint _((char *path, char *buf, int max));
int glue_fcntl _((int fd, int cmd, struct flock *));
int glue_ferror _((FILE *));
int glue_fflush _((FILE *));
int glue_fgetc _((FILE *));
FILE *glue_fopen _((char *path, char *mode));
int glue_fputc _((int, FILE *));
char *glue_getcwd _((char *buf, int max));
int glue_link _((char *p1, char *p2));
int glue_lstat _((char *path, struct stat *st));
int glue_mkdir _((char *path, int mode));
int glue_open _((char *path, int mode, int perm));
long glue_pathconf _((char *path, int mode));
int glue_readlink _((char *path, char *buf, int max));
int glue_read_whole_dir _((char *path, char **data, long *datalen));
int glue_rename _((char *p1, char *p2));
int glue_rmdir _((char *path));
int glue_rmdir_bg _((char *path));
int glue_rmdir_tree _((char *path));
int glue_stat _((char *path, struct stat *st));
int glue_symlink _((char *name1, char *name2));
int glue_ungetc _((int, FILE *));
int glue_unlink _((char *path));
int glue_utime _((char *path, struct utimbuf *));
int glue_write _((int fd, char *data, long len));
int glue_fwrite _((char *, long, long, FILE *));


#ifndef CONF_NO_seteuid
#ifndef aegis_glue_disable

/*
 * when seteuid works,
 * use the functions directly,
 * not the glue functions
 */
#define	glue_access	access
#define	glue_catfile	catfile
#define	glue_chmod	chmod
#define	glue_chown	chown
#define	glue_close	close
#define	glue_creat	creat
#define	glue_copyfile	copyfile
#define	glue_fclose	fclose
#define	glue_fcntl	fcntl
#define	glue_ferror	ferror
#define	glue_fflush	fflush
#define	glue_fgetc	getc
#define glue_file_compare file_compare
#define glue_file_fingerprint file_fingerprint
#define	glue_fopen	fopen
#define	glue_fputc	putc
#define	glue_getcwd	getcwd
#define	glue_link	link
#define	glue_lstat	lstat
#define	glue_mkdir	mkdir
#define	glue_open	open
#define	glue_pathconf	pathconf
#define	glue_readlink	readlink
#define	glue_read_whole_dir	read_whole_dir
#define	glue_rename	rename
#define	glue_rmdir	rmdir
#define	glue_rmdir_bg	rmdir_bg
#define	glue_rmdir_tree	rmdir_tree
#define	glue_stat	stat
#define	glue_symlink	symlink
#define	glue_ungetc	ungetc
#define	glue_unlink	unlink
#define	glue_utime	utime
#define	glue_write	write
#define	glue_fwrite	fwrite

#endif /* aegis_glue_disable */
#endif /* CONF_NO_seteuid */

#endif /* AEGIS_GLUE_H */
