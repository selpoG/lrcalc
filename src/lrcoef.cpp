#include "lrcalc/lrcoef.hpp"

#include <assert.h>
#include <stdint.h>

#include <memory>
#include <new>

#include "lrcalc/ivector.hpp"
#include "lrcalc/part.hpp"

struct lrcoef_box
{
	int value;     /* integer in box of skew tableau */
	int max;       /* upper bound for integer in box */
	int north;     /* index of box above */
	int east;      /* index of box to the right */
	int se_supply; /* number of available integers larger than value */
	int se_sz;     /* number of boxes to the right and strictly below */
	int west_sz;   /* number of boxes strictly to the left in same row */
	int padding;   /* make size a power of 2, improves speed in x86_64 */
};

struct lrcoef_content
{
	int cont;   /* number of boxes containing a given integer */
	int supply; /* total supply of given integer */
	lrcoef_content() : cont(0), supply(0) {}
};

static std::unique_ptr<lrcoef_content[]> lrcoef_new_content(const ivector* mu)
{
	assert(part_valid(mu));
	assert(part_length(mu) > 0);

	uint32_t n = part_length(mu);
	std::unique_ptr<lrcoef_content[]> res{new (std::nothrow) lrcoef_content[n + 1]()};
	if (!res) return res;
	res[0].cont = iv_elem(mu, 0);
	res[0].supply = iv_elem(mu, 0);
	for (uint32_t i = 0; i < n; i++) res[i + 1].supply = iv_elem(mu, i);
	return res;
}

static std::unique_ptr<lrcoef_box[]> lrcoef_new_skewtab(const ivector* nu, const ivector* la, int max_value)
{
	assert(part_valid(nu));
	assert(part_valid(la));
	assert(part_leq(la, nu));
	assert(part_entry(nu, 0) > 0);

	auto N = uint32_t(iv_sum(nu) - iv_sum(la));
	std::unique_ptr<lrcoef_box[]> array{new (std::nothrow) lrcoef_box[N + 2]};
	if (!array) return array;

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
			lrcoef_box* box = array.get() + --pos;
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
				box->max = array[uint32_t(below)].max - 1;
				box->se_sz = array[uint32_t(below)].se_sz + nu_1 - c;
			}
		}
	}
	array[N].value = 0;
	array[N + 1].value = max_value;
	array[N + 1].se_supply = 0;
	return array;
}

/* This is a low level function called from schur_lrcoef(). */
long long lrcoef_count(const ivector* outer, const ivector* inner, const ivector* content)
{
	assert(iv_sum(outer) == iv_sum(inner) + iv_sum(content));
	assert(iv_sum(content) > 1);

	auto T = lrcoef_new_skewtab(outer, inner, int(part_length(content)));
	if (!T) return -1;
	auto C = lrcoef_new_content(content);
	if (!C) return -1;

	int N = iv_sum(content);
	int pos = 0;
	lrcoef_box* box = T.get();
	int above = T[uint32_t(box->north)].value;
	int x = 1;
	int se_supply = N - C[1].supply;
	long long coef = 0;

	while (true)
	{
		while (x > above &&
		       (C[uint32_t(x)].cont == C[uint32_t(x)].supply || C[uint32_t(x)].cont == C[uint32_t(x - 1)].cont))
		{
			se_supply += (C[uint32_t(x)].supply - C[uint32_t(x)].cont);
			x--;
		}

		if (x == above || N - pos - se_supply <= box->west_sz)
		{
			pos--;
			if (pos < 0) break;
			box--;
			se_supply = box->se_supply;
			above = T[uint32_t(box->north)].value;
			x = box->value;
			C[uint32_t(x)].cont--;
			se_supply += (C[uint32_t(x)].supply - C[uint32_t(x)].cont);
			x--;
		}
		else if (pos + 1 < N)
		{
			box->se_supply = se_supply;
			box->value = x;
			C[uint32_t(x)].cont++;
			pos++;
			box++;
			se_supply = T[uint32_t(box->east)].se_supply;
			x = T[uint32_t(box->east)].value;
			above = T[uint32_t(box->north)].value;
			while (x > box->max)
			{
				se_supply += (C[uint32_t(x)].supply - C[uint32_t(x)].cont);
				x--;
			}
			while (x > above && se_supply < box->se_sz)
			{
				se_supply += (C[uint32_t(x)].supply - C[uint32_t(x)].cont);
				x--;
			}
		}
		else
		{
			coef++;
			pos--;
			box--;
			se_supply = box->se_supply;
			above = T[uint32_t(box->north)].value;
			x = box->value;
			C[uint32_t(x)].cont--;
			se_supply += (C[uint32_t(x)].supply - C[uint32_t(x)].cont);
			x--;
		}
	}

	return coef;
}
