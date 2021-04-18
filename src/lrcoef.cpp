#include "lrcalc/lrcoef.hpp"
#include "lrcalc/ivector.hpp"
#include "lrcalc/part.hpp"

typedef struct
{
	int value;     /* integer in box of skew tableau */
	int max;       /* upper bound for integer in box */
	int north;     /* index of box above */
	int east;      /* index of box to the right */
	int se_supply; /* number of available integers larger than value */
	int se_sz;     /* number of boxes to the right and strictly below */
	int west_sz;   /* number of boxes strictly to the left in same row */
	int padding;   /* make size a power of 2, improves speed in x86_64 */
} lrcoef_box;

typedef struct
{
	int cont;   /* number of boxes containing a given integer */
	int supply; /* total supply of given integer */
} lrcoef_content;

static lrcoef_content* lrcoef_new_content(ivector* mu)
{
	lrcoef_content* res;
	int n, i;

	claim(part_valid(mu));
	claim(part_length(mu) > 0);

	n = part_length(mu);
	res = (lrcoef_content*)ml_calloc(n + 1, sizeof(lrcoef_content));
	if (res == NULL) return NULL;
	res[0].cont = iv_elem(mu, 0);
	res[0].supply = iv_elem(mu, 0);
	for (i = 0; i < n; i++) res[i + 1].supply = iv_elem(mu, i);
	return res;
}

static lrcoef_box* lrcoef_new_skewtab(ivector* nu, ivector* la, int max_value)
{
	lrcoef_box* array;
	int N, pos, r, c;

	claim(part_valid(nu));
	claim(part_valid(la));
	claim(part_leq(la, nu));
	claim(part_entry(nu, 0) > 0);

	N = iv_sum(nu) - iv_sum(la);
	array = (lrcoef_box*)ml_malloc((N + 2) * sizeof(lrcoef_box));
	if (array == NULL) return NULL;

	pos = N;
	for (r = iv_length(nu) - 1; r >= 0; r--)
	{
		int nu_0 = (r == 0) ? iv_elem(nu, 0) : iv_elem(nu, r - 1);
		int la_0 = (r == 0) ? iv_elem(nu, 0) : part_entry(la, r - 1);
		int nu_r = iv_elem(nu, r);
		int la_r = part_entry(la, r);
		int nu_1 = part_entry(nu, r + 1);
		for (c = la_r; c < nu_r; c++)
		{
			lrcoef_box* box = array + --pos;
			box->north = (la_0 <= c && c < nu_0) ? pos - nu_r + la_0 : N;
			box->east = (c + 1 < nu_r) ? pos - 1 : N + 1;
			box->west_sz = c - la_r;
			if (c >= nu_1)
			{
				box->max = max_value;
				box->se_sz = 0;
			}
			else
			{
				int below = pos + nu_1 - la_r;
				box->max = array[below].max - 1;
				box->se_sz = array[below].se_sz + nu_1 - c;
			}
		}
	}
	array[N].value = 0;
	array[N + 1].value = max_value;
	array[N + 1].se_supply = 0;
	return array;
}

#ifdef DEBUG
void dump_content(lrcoef_content* C, int n)
{
	int i;
	printf("cont:");
	for (i = 0; i < n; i++) printf(" %d", C[i].cont);
	printf("  supply:");
	for (i = 0; i < n; i++) printf(" %d", C[i].supply);
	putchar('\n');
}

void dump_skewtab(lrcoef_box* T, int n)
{
	int i;
	printf("id: vl mx no es sp ss ws\n");
	for (i = 0; i < n; i++)
	{
		lrcoef_box* b = T + i;
		printf("%2d: %2d %2d %2d %2d %2d %2d %2d\n", i, b->value, b->max, b->north, b->east, b->se_supply, b->se_sz,
		       b->west_sz);
	}
}
#endif

/* This is a low level function called from schur_lrcoef(). */
long long lrcoef_count(ivector* outer, ivector* inner, ivector* content)
{
	lrcoef_content* C;
	lrcoef_box *T, *box;
	int N, pos, x, above, se_supply;
	long long coef;

	claim(iv_sum(outer) == iv_sum(inner) + iv_sum(content));
	claim(iv_sum(content) > 1);

	T = lrcoef_new_skewtab(outer, inner, part_length(content));
	if (T == NULL) return -1;
	C = lrcoef_new_content(content);
	if (C == NULL)
	{
		ml_free(T);
		return -1;
	}

	N = iv_sum(content);
	pos = 0;
	box = T;
	above = T[box->north].value;
	x = 1;
	se_supply = N - C[1].supply;
	coef = 0;

	while (1)
	{
		while (x > above && (C[x].cont == C[x].supply || C[x].cont == C[x - 1].cont))
		{
			se_supply += (C[x].supply - C[x].cont);
			x--;
		}

		if (x == above || N - pos - se_supply <= box->west_sz)
		{
			pos--;
			if (pos < 0) break;
			box--;
			se_supply = box->se_supply;
			above = T[box->north].value;
			x = box->value;
			C[x].cont--;
			se_supply += (C[x].supply - C[x].cont);
			x--;
		}
		else if (pos + 1 < N)
		{
			box->se_supply = se_supply;
			box->value = x;
			C[x].cont++;
			pos++;
			box++;
			se_supply = T[box->east].se_supply;
			x = T[box->east].value;
			above = T[box->north].value;
			while (x > box->max)
			{
				se_supply += (C[x].supply - C[x].cont);
				x--;
			}
			while (x > above && se_supply < box->se_sz)
			{
				se_supply += (C[x].supply - C[x].cont);
				x--;
			}
		}
		else
		{
			coef++;
			pos--;
			box--;
			se_supply = box->se_supply;
			above = T[box->north].value;
			x = box->value;
			C[x].cont--;
			se_supply += (C[x].supply - C[x].cont);
			x--;
		}
	}

	ml_free(T);
	ml_free(C);
	return coef;
}
