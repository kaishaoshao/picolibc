/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * and/or other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
FUNCTION
<<fopen64>>---open a large file

INDEX
	fopen64
INDEX
	_fopen64_r

SYNOPSIS
	#include <stdio.h>
	FILE *fopen64(const char *<[file]>, const char *<[mode]>);
	FILE *_fopen64_r(void *<[reent]>,
                       const char *<[file]>, const char *<[mode]>);

DESCRIPTION
<<fopen64>> is identical to <<fopen>> except it opens a large file that
is potentially >2GB in size.  See <<fopen>> for further details.

RETURNS
<<fopen64>> return a file pointer which you can use for other file
operations, unless the file you requested could not be opened; in that
situation, the result is <<NULL>>.  If the reason for failure was an
invalid string at <[mode]>, <<errno>> is set to <<EINVAL>>.

PORTABILITY
<<fopen64>> is a glibc extension.

Supporting OS subroutines required: <<close>>, <<fstat64>>, <<isatty>>,
<<lseek64>>, <<open64>>, <<read>>, <<sbrk>>, <<write>>.
*/

/* Copied from fopen.c */

#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/lock.h>
#include "../stdio/local.h"

#ifdef __LARGE64_FILES

FILE *
fopen64 (
	const char *file,
	const char *mode)
{
  register FILE *fp;
  register int f;
  int flags, oflags;

  if ((flags = __sflags (mode, &oflags)) == 0)
    return NULL;
  if ((fp = __sfp ()) == NULL)
    return NULL;

  if ((f = open64 (file, oflags, 0666)) < 0)
    {
      _newlib_sfp_lock_start ();
      fp->_flags = 0;		/* release */
      __lock_close_recursive (fp->_lock);
      _newlib_sfp_lock_end ();
      return NULL;
    }

  _newlib_flockfile_start (fp);

  fp->_file = f;
  fp->_flags = flags;
  fp->_cookie = (void *) fp;
  fp->_read = __sread;
  fp->_write = __swrite64;
  fp->_seek = __sseek;
  fp->_seek64 = __sseek64;
  fp->_close = __sclose;

  if (fp->_flags & __SAPP)
    fseeko64 (fp, 0, SEEK_END);

#ifdef __SCLE
  if (__stextmode (fp->_file))
    fp->_flags |= __SCLE;
#endif

  fp->_flags |= __SL64;

  _newlib_flockfile_end (fp);
  return fp;
}

#endif /* __LARGE64_FILES */
