/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
extern int optind;

#include "alloc.h"
#include "ivector.h"
#include "vectarg.h"

ivector* get_vect_arg(int ac, char** av)
{
	int n, i, x;
	int* tmp;
	ivector* res;
	char* endptr;
	char ch;

	if (optind == ac) return NULL;

	if (optind == 0) { optind++; }
	else
	{
		/* skip any "-" or "/" argument */
		ch = *(av[optind]);
		if ((ch == '-' || ch == '/') && *(av[optind] + 1) == '\0') optind++;
	}

	tmp = ml_malloc((ac - optind) * sizeof(int));
	if (tmp == NULL) return NULL;
	n = 0;

	while (optind < ac)
	{
		x = strtol(av[optind], &endptr, 10);
		if (endptr == av[optind] || *endptr != '\0') break;

		tmp[n++] = x;
		optind++;
	}

	if (n == 0) return NULL;

	res = iv_new(n);
	if (res == NULL) return NULL;
	for (i = 0; i < n; i++) iv_elem(res, i) = tmp[i];
	ml_free(tmp);

	return res;
}
