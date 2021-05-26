/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include "lrcalc/optshape.hpp"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <utility>

#include "lrcalc/cpp_lib.hpp"
#include "lrcalc/ivector.hpp"
#include "lrcalc/part.hpp"

skew_shape optim_fusion(const ivector* sh1, const ivector* sh2, int maxrows, int maxcols)
{
	skew_shape ss;
	_optim_fusion(&ss, sh1, sh2, maxrows, maxcols);
	return ss;
}

skew_shape optim_skew(const ivector* outer, const ivector* inner, const ivector* content, int maxrows)
{
	skew_shape ss;
	_optim_skew(&ss, outer, inner, content, maxrows);
	return ss;
}

skew_shape optim_coef(const ivector* out, const ivector* sh1, const ivector* sh2)
{
	skew_shape ss;
	_optim_coef(&ss, out, sh1, sh2);
	return ss;
}

/* Find optimal shape for fusion product. */

void _optim_fusion(skew_shape* ss, const ivector* sh1, const ivector* sh2, int rows, int level)
{
	/* DEBUG: Check valid input. */
	assert(part_valid(sh1));
	assert(part_valid(sh2));
	assert(part_entry(sh1, 0) - part_entry(sh1, rows - 1) <= level);
	assert(part_entry(sh2, 0) - part_entry(sh2, rows - 1) <= level);

	/* Empty result? */
	memset(ss, 0, sizeof(skew_shape));
	if (int(part_length(sh1)) > rows || int(part_length(sh2)) > rows) return;

	/* Find Seidel shift that results in smallest LHS partition. */
	int d1 = 0, d2 = 0, s1, s2;
	s1 = s2 = rows * level;
	for (int d = 1; d <= rows; d++)
	{
		int s = (rows - d) * level - rows * part_entry(sh1, d - 1);
		if (s < s1)
		{
			d1 = d;
			s1 = s;
		}
		s = (rows - d) * level - rows * part_entry(sh2, d - 1);
		if (s < s2)
		{
			d2 = d;
			s2 = s;
		}
	}
	if (s1 > s2)
	{
		std::swap(sh1, sh2);
		d1 = d2;
	}
	int d = d1;
	int sh1d = part_entry(sh1, d - 1);

	/* Create shifted partitions. */
	iv_ptr nsh1 = iv_create(uint32_t(rows));
	for (int i = 0; i < rows - d; i++) iv_elem(nsh1, i) = part_entry(sh1, d + i) - sh1d + level;
	for (int i = 0; i < d; i++) iv_elem(nsh1, rows - d + i) = part_entry(sh1, i) - sh1d;
	iv_ptr nsh2 = iv_create(uint32_t(rows));
	for (int i = 0; i < d; i++) iv_elem(nsh2, i) = part_entry(sh2, rows - d + i) + sh1d;
	for (int i = 0; i < rows - d; i++) iv_elem(nsh2, d + i) = part_entry(sh2, i) + sh1d - level;

	ss->outer = nsh1.release();
	ss->cont = nsh2.release();
	ss->sign = 1;
	return;
}

struct partial_shape
{
	ivector* inn;
	ivector* out; /* Build skew shape out/inn. */
	int rows;     /* Max column height. */
	int top;
	int bot; /* Left edge of skew shape is (top,col)...(bot-1,col). */
	int col;
	partial_shape(ivector* inn, ivector* out, int rows) : inn(inn), out(out), rows(rows), top(0), bot(0), col(0) {}
	void add_comp(const ivector* out0, const ivector* inn0, int c0, int r0t, int r0b, int c1, int r1t, int r1b)
	{
		int x = this->top + this->rows + r1t - r1b;
		if (x > this->bot) x = this->bot;
		int y1 = x + r1b - r1t;
		int z = y1 + r0b - r1b;

		int r;
		for (r = this->bot; r < y1; r++) iv_elem(this->out, r) = this->col;
		for (; r < z; r++)
		{
			int c = iv_elem(out0, r - x + r1t);
			iv_elem(this->out, r) = this->col + c - c1;
		}

		int len0 = (inn0 == nullptr) ? 0 : int(iv_length(inn0));
		int y0 = x + r0t - r1t;
		for (r = x; r < y0; r++)
		{
			int ra = r - x + r1t;
			int c = (ra < len0) ? iv_elem(inn0, ra) : 0;
			iv_elem(this->inn, r) = this->col + c - c1;
		}
		for (; r < z; r++) iv_elem(this->inn, r) = this->col - c1 + c0;

		this->col -= (c1 - c0);
		this->top = y0;
		this->bot = z;
	}
};

/* Find optimal shape for skew Schur function.
 *
 * 1) Check if outer/inner or content has column of height > maxrows.
 *
 * 2) Remove columns of height maxrows from outer/inner and content.
 *
 * 3) Separate outer/inner into independent components.
 *
 * 4) New content should be largest (anti) partition shaped component
 *    plus all columns of height maxrows.
 */

void _optim_skew(skew_shape* ss, const ivector* outer, const ivector* inner, const ivector* content, int maxrows)
{
	/* Handle case in other function. */
	if (inner == nullptr)
	{
		*ss = optim_mult(outer, content, maxrows, -1);
		return;
	}

	/* DEBUG: Check valid input. */
	assert(part_valid(outer));
	assert(part_valid(inner));
	if (content != nullptr) assert(part_valid(content));

	/* Indicate empty result. */
	memset(ss, 0, sizeof(skew_shape));
	if (!part_leq(inner, outer)) return;

	/* Find range of non-empty rows in outer/inner. */
	auto row_bound = int(part_length(outer));
	int ilen = (inner == nullptr) ? 0 : int(iv_length(inner));
	if (row_bound <= ilen)
	{
		while (row_bound > 0 && iv_elem(inner, row_bound - 1) == iv_elem(outer, row_bound - 1)) row_bound--;
		ilen = row_bound;
	}
	int row_first = 0;
	while (row_first < ilen && iv_elem(inner, row_first) == iv_elem(outer, row_first)) row_first++;
	int row_span = row_bound - row_first;

	/* Bound number of rows in content of LR tableaux. */
	int clen = (content == nullptr) ? 0 : int(part_length(content));
	if (maxrows >= 0 && clen > maxrows) return;
	/* FIXME: Prove slen is large enough!!! */
	int slen = 2 * row_span + clen;
	if (maxrows < 0) maxrows = slen + 1;

	/* Allocate new skew shape. */
	iv_ptr out = iv_create(uint32_t(slen));
	iv_ptr inn = iv_create(uint32_t(slen));

	/* Allocate and copy content. */
	iv_ptr cont = iv_create(uint32_t((clen > row_span) ? clen : row_span));
	int cont_size = 0;
	for (int r = clen - 1; r >= 0; r--)
	{
		iv_elem(cont, r) = iv_elem(content, r);
		cont_size += iv_elem(content, r);
	}

	/* Empty shape outer/inner ? */
	if (row_bound == 0)
	{
		iv_length(inn) = 0;
		iv_length(out) = 0;
		iv_length(cont) = uint32_t(clen);
		ss->outer = out.release();
		ss->inner = inn.release();
		ss->cont = cont.release();
		ss->sign = 1;
		return;
	}

	/* Number of columns of size maxrows. */
	int full_cols = 0;
	if (clen == maxrows && maxrows > 0)
	{
		full_cols = iv_elem(cont, clen - 1);
		for (int r = clen - 1; r >= 0; r--) iv_elem(cont, r) -= full_cols;
		cont_size -= full_cols * clen;
	}

	/* Find component with upper-right (r2t,c2-1), lower-right (r2b-1,c2-1). */
	int c2 = iv_elem(outer, row_first);
	int r2t = row_first;
	int r2b = row_first;
	while (r2b < row_bound && c2 <= iv_elem(outer, r2b)) r2b++;

	/* Find component with upper-left (r1t,c1), lower-left (r1b-1,c1). */
	int c1 = c2;
	int r0t = r2t;
	int r0b = r2b;

	/* Skew shape structure to pass to add_comp. */
	partial_shape ps{inn.release(), out.release(), maxrows};

	for (c1--; c1 >= 0; c1--)
	{
		/* Find row range for column c1-1. */
		int r1t = r0t;
		int r1b = r0b;
		if (c1 == 0) r0t = r0b = row_bound;
		while (r0b < row_bound && c1 <= iv_elem(outer, r0b)) r0b++;
		while (r0t < int(iv_length(inner)) && c1 <= iv_elem(inner, r0t)) r0t++;

		/* No new component? */
		if (r0t < r1b && r0b - r1t < maxrows) continue;

		/* Single column too high? */
		if (c1 == c2 - 1 && r1b - r1t > maxrows) return;

		/* Single column of full height? */
		if (c1 == c2 - 1 && r1b - r1t == maxrows)
		{
			full_cols++;
			c2 = c1;
			r2t = r0t;
			r2b = r0b;
			continue;
		}

		/* Find size of component. */
		int comp_size = 0;
		for (int r = r2t; r < r1b; r++)
		{
			int a, b;
			a = (r < int(iv_length(inner))) ? iv_elem(inner, r) : 0;
			if (a < c1) a = c1;
			b = iv_elem(outer, r);
			if (b > c2) b = c2;
			comp_size += (b - a);
		}

		if ((r1t == r2t || r1b == r2b) && 0 < cont_size && cont_size < comp_size)
		{
			/* Add content as component. */
			int r = 1;
			int c = iv_elem(cont, 0);
			while (r < clen && iv_elem(cont, r) == c) r++;
			ps.add_comp(cont.get(), nullptr, 0, 0, clen, c, 0, r);
		}

		if (r1t == r2t && comp_size > cont_size)
		{
			/* Component of larger partition shape. */
			clen = r1b - r1t;
			int r;
			for (r = r1t; r < r2b; r++) iv_elem(cont, r - r1t) = c2 - c1;
			for (; r < r1b; r++) iv_elem(cont, r - r1t) = iv_elem(outer, r) - c1;
			cont_size = comp_size;
		}
		else if (r1b == r2b && comp_size > cont_size)
		{
			/* Component of larger anti-partition shape. */
			clen = r2b - r2t;
			int r;
			for (r = r2b - 1; r >= r1t; r--) iv_elem(cont, r2b - 1 - r) = c2 - c1;
			for (; r >= r2t; r--) iv_elem(cont, r2b - 1 - r) = c2 - iv_elem(inner, r);
			cont_size = comp_size;
		}
		else if (comp_size > 0)
			ps.add_comp(outer, inner, c1, r1t, r1b, c2, r2t, r2b);

		c2 = c1;
		r2t = r0t;
		r2b = r0b;
	}

	if (full_cols)
	{
		int r;
		for (r = 0; r < clen; r++) iv_elem(cont, r) += full_cols;
		for (; r < maxrows; r++) iv_elem(cont, r) = full_cols;
		clen = maxrows;
	}
	iv_length(cont) = uint32_t(clen);

	iv_length(ps.out) = uint32_t(ps.bot);
	iv_length(ps.inn) = uint32_t(ps.bot);
	for (int r = ps.bot - 1; r >= 0; r--)
	{
		iv_elem(ps.out, r) -= ps.col;
		iv_elem(ps.inn, r) -= ps.col;
	}

	ss->outer = ps.out;
	ss->inner = ps.inn;
	ss->cont = cont.release();
	ss->sign = 1;
	return;
}

void _optim_coef(skew_shape* ss, const ivector* out, const ivector* sh1, const ivector* sh2)
{
	// int N, Nla, Nmu, sum, r, s, N0, nu0, la0, mu0, nur, lar, mur;
	// int lar1, mur1, nur1, c, ca, Inu, Ila, Imu;

	assert(part_valid(out));
	assert(part_valid(sh1));
	assert(part_valid(sh2));

	memset(ss, 0, sizeof(skew_shape));

	int N = int(part_length(out));
	if (N < int(iv_length(sh1)) && iv_elem(sh1, N) > 0) return;
	if (N < int(iv_length(sh2)) && iv_elem(sh2, N) > 0) return;
	if (N == 0)
	{
		ss->sign = 1;
		return;
	}

	iv_ptr nu = iv_create(uint32_t(N));
	iv_ptr la = iv_create(uint32_t(N));
	iv_ptr mu = iv_create(uint32_t(N));

	int sum = 0;
	for (int r = N - 1; r >= 0; r--)
	{
		iv_elem(nu, r) = iv_elem(out, r);
		sum += iv_elem(nu, r);
	}

	int Nla;
	for (Nla = N; Nla > int(iv_length(sh1)); Nla--) iv_elem(la, Nla - 1) = 0;
	for (; Nla > 0 && iv_elem(sh1, Nla - 1) == 0; Nla--) iv_elem(la, Nla - 1) = 0;
	for (int r = Nla - 1; r >= 0; r--)
	{
		int x = iv_elem(sh1, r);
		iv_elem(la, r) = x;
		if (iv_elem(nu, r) < x) return;
		sum -= iv_elem(la, r);
	}

	int Nmu;
	for (Nmu = N; Nmu > int(iv_length(sh2)); Nmu--) iv_elem(mu, Nmu - 1) = 0;
	for (; Nmu > 0 && iv_elem(sh2, Nmu - 1) == 0; Nmu--) iv_elem(mu, Nmu - 1) = 0;
	for (int r = Nmu - 1; r >= 0; r--)
	{
		int x = iv_elem(sh2, r);
		iv_elem(mu, r) = x;
		if (iv_elem(nu, r) < x) return;
		sum -= iv_elem(mu, r);
	}

	if (sum != 0) return;

	int N0 = N + 1;
	int nu0 = 0;
	while (N < N0 || iv_elem(nu, 0) < nu0)
	{
		N0 = N;
		nu0 = iv_elem(nu, 0);

		/* Horizontal compactification of nu/la. */
		int mu0 = iv_elem(mu, 0);
		int lar1 = 0;
		int nur1 = 0;
		int r;
		for (r = N - 1; r >= 0; r--)
		{
			int lar = iv_elem(la, r);
			int nur = iv_elem(nu, r);
			if (lar > nur1 || nur - lar1 > mu0) break;
			lar1 = lar;
			nur1 = nur;
		}
		int c = 0;
		for (; r >= 0; r--)
		{
			int lar = iv_elem(la, r);
			int nur = iv_elem(nu, r);
			if (nur - lar > mu0) return;
			int ca = nur - lar1 - mu0;
			if (ca < lar - nur1) ca = lar - nur1;
			if (ca > 0) c += ca;
			if (nur - c < iv_elem(mu, r)) return;
			if (nur == c)
			{
				N = r;
				break;
			}
			iv_elem(la, r) = lar - c;
			iv_elem(nu, r) = nur - c;
			lar1 = lar;
			nur1 = nur;
		}

		/* Remove row of size mu[0] from nu/la. */
		r = 0;
		while (r < N && iv_elem(nu, r) - iv_elem(la, r) < mu0) r++;
		if (r < N)
		{
			if (iv_elem(nu, r) - iv_elem(la, r) > mu0) return;
			for (; r < N - 1; r++)
			{
				iv_elem(la, r) = iv_elem(la, r + 1);
				iv_elem(nu, r) = iv_elem(nu, r + 1);
			}
			for (r = 0; r < N - 1; r++) iv_elem(mu, r) = iv_elem(mu, r + 1);
			N -= 1;
		}

		/* Horizontal compactification of nu/mu. */
		int la0 = iv_elem(la, 0);
		int mur1 = 0;
		nur1 = 0;
		for (r = N - 1; r >= 0; r--)
		{
			int mur = iv_elem(mu, r);
			int nur = iv_elem(nu, r);
			if (mur > nur1 || nur - mur1 > la0) break;
			mur1 = mur;
			nur1 = nur;
		}
		c = 0;
		for (; r >= 0; r--)
		{
			int mur = iv_elem(mu, r);
			int nur = iv_elem(nu, r);
			if (nur - mur > la0) return;
			int ca = nur - mur1 - la0;
			if (ca < mur - nur1) ca = mur - nur1;
			if (ca > 0) c += ca;
			if (nur - c < iv_elem(la, r)) return;
			if (nur == c)
			{
				N = r;
				break;
			}
			iv_elem(mu, r) = mur - c;
			iv_elem(nu, r) = nur - c;
			mur1 = mur;
			nur1 = nur;
		}

		/* Remove row of size la[0] from nu/la. */
		r = 0;
		while (r < N && iv_elem(nu, r) - iv_elem(mu, r) < la0) r++;
		if (r < N)
		{
			if (iv_elem(nu, r) - iv_elem(mu, r) > la0) return;
			for (; r < N - 1; r++)
			{
				iv_elem(mu, r) = iv_elem(mu, r + 1);
				iv_elem(nu, r) = iv_elem(nu, r + 1);
			}
			for (r = 0; r < N - 1; r++) iv_elem(la, r) = iv_elem(la, r + 1);
			N -= 1;
		}

		/* Vertical compactification of nu/la. */
		if (N < Nmu) Nmu = N;
		while (Nmu > 0 && iv_elem(mu, Nmu - 1) == 0) Nmu--;
		if (Nmu == 0) goto coef_one;
		r = 0;
		while (r < Nmu && iv_elem(la, r) < iv_elem(nu, r)) r++;
		while (r < N && iv_elem(la, r) < iv_elem(nu, r) && iv_elem(nu, r) < iv_elem(la, r - Nmu)) r++;
		if (r < N)
		{
			int Inu = r;
			int s = (r > Nmu) ? (r - Nmu) : 0;
			int Ila = s;
			for (; r < N && Inu < Nmu; r++)
			{
				if (iv_elem(la, r) == iv_elem(nu, r))
					iv_elem(la, r) = -1;
				else
				{
					iv_elem(nu, Inu) = iv_elem(nu, r);
					if (iv_elem(nu, Inu) < iv_elem(mu, Inu)) return;
					Inu++;
				}
			}
			while (r < N)
			{
				if (iv_elem(la, r) == iv_elem(nu, r))
				{
					iv_elem(la, r) = -1;
					r++;
					continue;
				}
				while (iv_elem(la, s) == -1) s++;
				if (iv_elem(la, s) < iv_elem(nu, r)) return;
				if (iv_elem(la, s) > iv_elem(nu, r))
				{
					iv_elem(la, Ila) = iv_elem(la, s);
					Ila++;
					iv_elem(nu, Inu) = iv_elem(nu, r);
					if (iv_elem(nu, Inu) < iv_elem(mu, Inu)) return;
					Inu++;
				}
				r++;
				s++;
			}
			while (s < N)
			{
				if (iv_elem(la, s) != -1)
				{
					iv_elem(la, Ila) = iv_elem(la, s);
					Ila++;
				}
				s += 1;
			}
			if (Inu < N && iv_elem(mu, Inu) > 0) return;
			N = Inu;
		}

		/* Remove column of size len(mu) from nu/la. */
		r = Nmu;
		while (r <= N && iv_elem(nu, r - 1) <= iv_elem(la, r - Nmu)) r++;
		if (r <= N)
		{
			if (r > Nmu && iv_elem(nu, r - 1) > iv_elem(la, r - Nmu - 1)) return;
			if (r < N && iv_elem(nu, r) > iv_elem(la, r - Nmu)) return;
			int s;
			for (s = r - Nmu - 1; s >= 0; s--) iv_elem(la, s)--;
			for (s = Nmu - 1; s >= 0; s--) iv_elem(mu, s)--;
			for (s = 0; s < r; s++)
			{
				iv_elem(nu, s)--;
				if (iv_elem(nu, s) == 0)
				{
					N = s;
					break;
				}
			}
		}

		/* Vertical compactification of nu/mu. */
		if (N < Nla) Nla = N;
		while (Nla > 0 && iv_elem(la, Nla - 1) == 0) Nla--;
		if (Nla == 0) goto coef_one;
		r = 0;
		while (r < Nla && iv_elem(mu, r) < iv_elem(nu, r)) r++;
		while (r < N && iv_elem(mu, r) < iv_elem(nu, r) && iv_elem(nu, r) < iv_elem(mu, r - Nla)) r++;
		if (r < N)
		{
			int Inu = r;
			int s = (r > Nla) ? (r - Nla) : 0;
			int Imu = s;
			for (; r < N && Inu < Nla; r++)
			{
				if (iv_elem(mu, r) == iv_elem(nu, r))
					iv_elem(mu, r) = -1;
				else
				{
					iv_elem(nu, Inu) = iv_elem(nu, r);
					if (iv_elem(nu, Inu) < iv_elem(la, Inu)) return;
					Inu++;
				}
			}
			while (r < N)
			{
				if (iv_elem(mu, r) == iv_elem(nu, r))
				{
					iv_elem(mu, r) = -1;
					r++;
					continue;
				}
				while (iv_elem(mu, s) == -1) s++;
				if (iv_elem(mu, s) < iv_elem(nu, r)) return;
				if (iv_elem(mu, s) > iv_elem(nu, r))
				{
					iv_elem(mu, Imu) = iv_elem(mu, s);
					Imu++;
					iv_elem(nu, Inu) = iv_elem(nu, r);
					if (iv_elem(nu, Inu) < iv_elem(la, Inu)) return;
					Inu++;
				}
				r++;
				s++;
			}
			while (s < N)
			{
				if (iv_elem(mu, s) != -1)
				{
					iv_elem(mu, Imu) = iv_elem(mu, s);
					Imu++;
				}
				s += 1;
			}
			if (Inu < N && iv_elem(la, Inu) > 0) return;
			N = Inu;
		}

		/* Remove column of size len(la) from nu/mu. */
		r = Nla;
		while (r <= N && iv_elem(nu, r - 1) <= iv_elem(mu, r - Nla)) r++;
		if (r <= N)
		{
			if (r > Nla && iv_elem(nu, r - 1) > iv_elem(mu, r - Nla - 1)) return;
			if (r < N && iv_elem(nu, r) > iv_elem(mu, r - Nla)) return;
			int s;
			for (s = r - Nla - 1; s >= 0; s--) iv_elem(mu, s)--;
			for (s = Nla - 1; s >= 0; s--) iv_elem(la, s)--;
			for (s = 0; s < r; s++)
			{
				iv_elem(nu, s)--;
				if (iv_elem(nu, s) == 0)
				{
					N = s;
					break;
				}
			}
		}
	}

	if (N == 0) goto coef_one;

	iv_length(nu) = uint32_t(N);
	if (N < Nla) Nla = N;
	while (Nla > 0 && iv_elem(la, Nla - 1) == 0) Nla--;
	iv_length(la) = uint32_t(Nla);
	if (N < Nmu) Nmu = N;
	while (Nmu > 0 && iv_elem(mu, Nmu - 1) == 0) Nmu--;
	iv_length(mu) = uint32_t(Nmu);

	ss->outer = nu.release();
	ss->inner = la.release();
	ss->cont = mu.release();
	ss->sign = 2;
	return;

coef_one:
	ss->sign = 1;
	return;
}
