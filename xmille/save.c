/*	$NetBSD: save.c,v 1.11 2003/08/07 09:37:26 agc Exp $	*/

/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>

#include <time.h>

#include "mille.h"

#ifndef	unctrl
#include "unctrl.h"
#endif

/*
 * @(#)save.c	1.2 (Berkeley) 3/28/83
 */

typedef	struct stat	STAT;

/*
 *	This routine saves the current game for use at a later date
 *	Returns FALSE if it couldn't be done.
 */
bool
save(void)
{
	char	*sp, *ans;
	int	outf;
	time_t	*tp;
	char	buf[80];
	time_t	tme;
	STAT	junk;
	bool	rv;

	sp = NULL;
	tp = &tme;
	if (Fromfile && getyn("Same file? "))
		strcpy(buf, Fromfile);
	else {
		ans = GetpromptedInput ("File: ");
		if (!ans)
		    return FALSE;
		strcpy (buf, ans);
		sp = buf + strlen (buf);
	}

	/*
	 * check for existing files, and confirm overwrite if needed
	 */

	if (sp == buf || (!Fromfile && stat(buf, &junk) > -1
	    && getyn("Overwrite File? ") == FALSE))
		return FALSE;

	if ((outf = creat(buf, 0644)) < 0) {
		error(strerror(errno));
		return FALSE;
	}
	Error (buf);
	time(tp);			/* get current time		*/
	rv = varpush(outf, (ssize_t(*)(int, void *, size_t))write);
	close(outf);
	if (rv == FALSE)
		unlink(buf);
	return rv;
}

/*
 *	This does the actual restoring.  It returns TRUE if the
 * backup was made on exiting, in which case certain things must
 * be cleaned up before the game starts.
 */
bool
rest_f(const char *file)
{

	char	*sp;
	int	inf;
	char	buf[80];
	STAT	sbuf;

	if ((inf = open(file, O_RDONLY)) < 0) {
		warn("%s", file);
		exit(1);
	}
	if (fstat(inf, &sbuf) < 0) {		/* get file stats	*/
		warn("%s", file);
		exit(1);
	}
	varpush(inf, (ssize_t (*)(int, void *, size_t))read);
	close(inf);
	strcpy(buf, ctime(&sbuf.st_mtime));
	for (sp = buf; *sp != '\n'; sp++)
		continue;
	*sp = '\0';
	/*
	 * initialize some necessary values
	 */
	(void)sprintf(Initstr, "%s [%s]\n", file, buf);
	Fromfile = file;
	return !On_exit;
}

bool
rest(void)
{
	char	buf[80];
	char *ans;
	if (Fromfile && getyn("Same file? "))
		strcpy(buf, Fromfile);
	else {
		ans = GetpromptedInput ("File: ");
		if (!ans)
		    return FALSE;
		strcpy (buf, ans);
	}
	return rest_f(buf);
}
