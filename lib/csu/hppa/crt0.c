/*	$OpenBSD: crt0.c,v 1.8 2003/06/04 04:43:56 deraadt Exp $	*/

/*
 * Copyright (c) 2001 Michael Shalayeff
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR OR HIS RELATIVES BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF MIND, USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

int	global __asm ("$global$") = 0;
int	sh_func_adrs __asm ("$$sh_func_adrs") = 0;

#if defined(LIBC_SCCS) && !defined(lint)
static const char rcsid[] = "$OpenBSD: crt0.c,v 1.8 2003/06/04 04:43:56 deraadt Exp $";
#endif /* LIBC_SCCS and not lint */

#include <sys/param.h>
#include <sys/syscall.h>
#include <sys/fcntl.h>
#include <sys/exec.h>
#include <stdlib.h>
#include <paths.h>

typedef char Obj_Entry;

/*
 * Lots of the chunks of this file cobbled together from pieces of
 * other OpenBSD crt files, including the common code.
 */

char	**environ;

extern void	__init(void);
extern void	__fini(void);

#ifdef MCRT0
extern void	monstartup(u_long, u_long);
extern void	_mcleanup(void);
extern unsigned char etext, eprol;
#endif /* MCRT0 */

void __start(char **, void (*)(void), const Obj_Entry *);
static char *__strrchr(const char *p, char ch);

char *__progname = "";
char __progname_storage[NAME_MAX+1];

void
__start(sp, cleanup, obj)
	char **sp;
	void (*cleanup)(void);			/* from shared loader */
	const Obj_Entry *obj;			/* from shared loader */
{
	struct ps_strings *arginfo = (struct ps_strings *)sp;
	char **argv, *namep;
	char *s;

	__asm __volatile (".import $global$, data\n\t"
			  "ldil L%%$global$, %%r27\n\t"
			  "ldo	R%%$global$(%%r27), %%r27" ::: "r27");

	argv = arginfo->ps_argvstr;
	environ = arginfo->ps_envstr;
	if ((namep = argv[0]) != NULL) {	/* NULL ptr if argc = 0 */
		if ((__progname = __strrchr(namep, '/')) == NULL)
			__progname = namep;
		else
			__progname++;
		for (s = __progname_storage; *__progname &&
		    s < &__progname_storage[sizeof __progname_storage - 1]; )
			*s++ = *__progname++;
		*s = '\0';
		__progname = __progname_storage;
	}

#ifdef MCRT0
	atexit(_mcleanup);
	monstartup((u_long)&eprol, (u_long)&etext);
#endif

	__init();

	exit(main(arginfo->ps_nargvstr, argv, environ));
}

static char *
__strrchr(const char *p, char ch)
{
	char *save;

	for (save = NULL;; ++p) {
		if (*p == ch)
			save = (char *)p;
		if (!*p)
			return(save);
	}
}

#ifdef MCRT0
__asm (".export eprol, entry\n\t.label eprol");
#endif
