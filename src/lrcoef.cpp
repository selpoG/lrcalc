#include "lrcalc/lrcoef.hpp"
#include "lrcalc/ivector.hpp"
#include "lrcalc/part.hpp"

#include <assert.h>

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

static lrcoef_content* lrcoef_new_content(const ivector* mu)
{
	assert(part_valid(mu));
	assert(part_length(mu) > 0);

	uint32_t n = part_length(mu);
	lrcoef_content* res = static_cast<lrcoef_content*>(calloc(n + 1, sizeof(lrcoef_content)));
	if (res == nullptr) return nullptr;
	res[0].cont = iv_elem(mu, 0);
	res[0].supply = iv_elem(mu, 0);
	for (uint32_t i = 0; i < n; i++) res[i + 1].supply = iv_elem(mu, i);
	return res;
}

static lrcoef_box* lrcoef_new_skewtab(const ivector* nu, const ivector* la, int max_value)
{
	assert(part_valid(nu));
	assert(part_valid(la));
	assert(part_leq(la, nu));
	assert(part_entry(nu, 0) > 0);

	auto N = uint32_t(iv_sum(nu) - iv_sum(la));
	lrcoef_box* array = static_cast<lrcoef_box*>(malloc((N + 2) * sizeof(lrcoef_box)));
	if (array == nullptr) return nullptr;

	auto pos = int(N);
	for (int r = int(iv_length(nu)) - 1; r >= 0; r--)
	{
		int nu_0 = (r == 0) ? iv_elem(nu, 0) : iv_elem(nu, r - 1);
		int la_0 = (r == 0) ? iv_elem(nu, 0) : part_entry(la, r - 1);
		int nu_r = iv_elem(nu, r);
		int la_r = part_entry(la, r);
		int nu_1 = part_entry(nu, r + 1);
		for (int c = la_r; c < nu_r; c++)
		{
			lrcoef_box* box = array + --pos;
			box->north = (la_0 <= c && c < nu_0) ? pos - nu_r + la_0 : int(N);
			box->east = (c + 1 < nu_r) ? pos - 1 : int(N + 1);
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
void dump_content(const lrcoef_content* C, uint32_t n)
{
	printf("cont:");
	for (uint32_t i = 0; i < n; i++) printf(" %d", C[i].cont);
	printf("  supply:");
	for (uint32_t i = 0; i < n; i++) printf(" %d", C[i].supply);
	putchar('\n');
}

void dump_skewtab(const lrcoef_box* T, uint32_t n)
{
	printf("id: vl mx no es sp ss ws\n");
	for (uint32_t i = 0; i < n; i++)
	{
		lrcoef_box* b = T + i;
		printf("%2d: %2d %2d %2d %2d %2d %2d %2d\n", i, b->value, b->max, b->north, b->east, b->se_supply, b->se_sz,
		       b->west_sz);
	}
}
#endif

/* This is a low level function called from schur_lrcoef(). */
long long lrcoef_count(const ivector* outer, const ivector* inner, const ivector* content)
{
	assert(iv_sum(outer) == iv_sum(inner) + iv_sum(content));
	assert(iv_sum(content) > 1);

	lrcoef_box* T = lrcoef_new_skewtab(outer, inner, int(part_length(content)));
	if (T == nullptr) return -1;
	lrcoef_content* C = lrcoef_new_content(content);
	if (C == nullptr)
	{
		free(T);
		return -1;
	}

	int N = iv_sum(content);
	int pos = 0;
	lrcoef_box* box = T;
	int above = T[box->north].value;
	int x = 1;
	int se_supply = N - C[1].supply;
	long long coef = 0;

	while (true)
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

	free(T);
	free(C);
	return coef;
}
