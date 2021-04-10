/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include "ivector.h"
#include "ivlincomb.h"
#include "perm.h"

#define _SCHUBLIB_C
#include "schublib.h"


static int _trans(ivector *w, int vars, ivlincomb *res);

ivlincomb *trans(ivector *w, int vars)
{
  ivlincomb *res = ivlc_new(IVLC_HASHTABLE_SZ, IVLC_ARRAY_SZ);
  if (res == NULL)
    return NULL;
  if (_trans(w, vars, res) != 0)
    {
      ivlc_free_all(res);
      return NULL;
    }
  return res;
}

static int _trans(ivector *w, int vars, ivlincomb *res)
{
  ivlincomb *tmp;
  ivlc_iter itr;
  ivector *v;
  int n, nw, r, s, wr, ws, i, vi, vr, last;

  ivlc_reset(res);

  nw = iv_length(w);
  n = perm_group(w);
  w->length = n;

  r = n-1;
  while (r > 0 && iv_elem(w, r-1) < iv_elem(w, r))
    r--;
  if (r <= 0)
    {
      ivector *xx = iv_new_zero(vars ? vars : 1);
      w->length = nw;
      if (xx == NULL)
        return -1;
      if (ivlc_insert(res, xx, iv_hash(xx), 1) == NULL)
        {
          iv_free(xx);
          return -1;
        }
      return 0;
    }
  if (vars < r)
    vars = r;

  s = r + 1;
  while (s < n && iv_elem(w, r-1) > iv_elem(w, s))
    s++;

  wr = iv_elem(w, r-1);
  ws = iv_elem(w, s-1);

  v = w;
  iv_elem(v, s-1) = wr;
  iv_elem(v, r-1) = ws;

  tmp = trans(v, vars);
  if (tmp == NULL)
    {
      w->length = nw;
      return -1;
    }
  for (ivlc_first(tmp, &itr); ivlc_good(&itr); ivlc_next(&itr))
    {
      ivector *xx = ivlc_key(&itr);
      uint32_t hash;
      iv_elem(xx, r-1)++;
      hash = iv_hash(xx);
      if (ivlc_insert(res, xx, hash, ivlc_value(&itr)) == NULL)
        {
          ivlc_free_all(tmp);
          w->length = nw;
          return -1;
        }
    }

  last = 0;
  vr = iv_elem(v, r-1);
  for (i = r-1; i >= 1; i--)
    {
      vi = iv_elem(v, i-1);
      if (last < vi && vi < vr)
        {
          int ok;
          last = vi;
          iv_elem(v, i-1) = vr;
          iv_elem(v, r-1) = vi;
          ok = _trans(v, vars, tmp);
          if (ok == 0)
            ok = ivlc_add_multiple(res, 1, tmp, LC_FREE_ZERO);
          if (ok != 0)
            {
              ivlc_free_all(tmp);
              w->length = nw;
              return -1;
            }
          iv_elem(v, i-1) = vi;
        }
    }

  w->length = nw;
  iv_elem(w, s-1) = ws;
  iv_elem(w, r-1) = wr;
  ivlc_free(tmp);

  return 0;
}


static int _monk_add(int i, ivlincomb *slc, int rank, ivlincomb *res);

ivlincomb *monk(int i, ivlincomb *slc, int rank)
{
  ivlincomb *res = ivlc_new(IVLC_HASHTABLE_SZ, IVLC_ARRAY_SZ);
  if (res == NULL)
    return NULL;
  if (rank == 0)
    rank = (((unsigned) -1) >> 1);
  if (_monk_add(i, slc, rank, res) != 0)
    {
      ivlc_free_all(res);
      return NULL;
    }
  return res;
}

static int _monk_add(int i, ivlincomb *slc, int rank, ivlincomb *res)
{
  ivlc_iter itr;

  for (ivlc_first(slc, &itr); ivlc_good(&itr); ivlc_next(&itr))
    {
      ivector *u, *w = ivlc_key(&itr);
      int j, t;
      int c = ivlc_value(&itr);
      int n = iv_length(w);
      int wi = (i <= n) ? iv_elem(w, i-1) : i;

      if (i <= n+1)
        {
          int last, ulen = (i > n) ? i : n;
          last = 0;
          for (j = i-1; j >= 1; j--)
            if (last < iv_elem(w, j-1) && iv_elem(w, j-1) < wi)
              {
                last = iv_elem(w, j-1);
                u = iv_new(ulen);
                if (u == NULL)
                  return -1;
                for (t = 0; t < n; t++)
                  iv_elem(u, t) = iv_elem(w, t);
                for (t = n; t < ulen; t++)
                  iv_elem(u, t) = t+1;
                iv_elem(u, j-1) = wi;
                iv_elem(u, i-1) = last;
                if (ivlc_add_element(res, -c, u, iv_hash(u),
                                     LC_FREE_ZERO) != 0)
                  return -1;
              }
        }
      else
        {
          u = iv_new(i);
          if (u == NULL)
            return -1;
          for (t = 0; t < n; t++)
            iv_elem(u, t) = iv_elem(w, t);
          for (t = n; t < i-2; t++)
            iv_elem(u, t) = t+1;
          iv_elem(u, i-2) = i;
          iv_elem(u, i-1) = i-1;
          if (ivlc_add_element(res, -c, u, iv_hash(u), LC_FREE_ZERO) != 0)
            return -1;
        }

      if (i >= n+1)
        {
          u = iv_new(i + 1);
          if (u == NULL)
            return -1;
          for (t = 0; t < n; t++)
            iv_elem(u, t) = iv_elem(w, t);
          for (t = n; t < i; t++)
            iv_elem(u, t) = t+1;
          iv_elem(u, i-1) = i+1;
          iv_elem(u, i) = i;
          if (ivlc_add_element(res, c, u, iv_hash(u), LC_FREE_ZERO) != 0)
            return -1;
        }
      else
        {
          int last;
          last = (((unsigned) -1) >> 1);
          for (j = i+1; j <= n; j++)
            if (wi < iv_elem(w, j-1) && iv_elem(w, j-1) < last)
              {
                last = iv_elem(w, j-1);
                u = iv_new(n);
                if (u == NULL)
                  return -1;
                for (t = 0; t < n; t++)
                  iv_elem(u, t) = iv_elem(w, t);
                iv_elem(u, i-1) = last;
                iv_elem(u, j-1) = wi;
                if (ivlc_add_element(res, c, u, iv_hash(u),
                                     LC_FREE_ZERO) != 0)
                  return -1;
              }
          if (last > n && n < rank)
            {
              u = iv_new(n+1);
              if (u == NULL)
                return -1;
              for (t = 0; t < n; t++)
                iv_elem(u, t) = iv_elem(w, t);
              iv_elem(u, i-1) = n+1;
              iv_elem(u, n) = wi;
              if (ivlc_add_element(res, c, u, iv_hash(u), LC_FREE_ZERO) != 0)
                return -1;
            }
        }
    }
  return 0;
}


static int
_mult_ps(void **poly, int n, int maxvar, ivector *perm, int rank,
         ivlincomb *res);

ivlincomb *mult_poly_schubert(ivlincomb *poly, ivector *perm, int rank)
{
  ivlc_iter itr;
  void **p;
  int i, j, n, maxvar, svlen, ok;

  n = ivlc_card(poly);
  if (n == 0)
    return poly;

  if (rank == 0)
    rank = (((unsigned) -1) >> 1);

  p = (void **) ml_malloc(2 * n * sizeof(void *));
  if (p == NULL)
    {
      ivlc_free_all(poly);
      return NULL;
    }
  i = 0;
  maxvar = 0;
  for (ivlc_first(poly, &itr); ivlc_good(&itr); ivlc_next(&itr))
    {
      ivector *xx = ivlc_key(&itr);
      j = iv_length(xx);
      while (j > 0 && iv_elem(xx, j-1) == 0)
        j--;
      xx->length = j;
      if (maxvar < j)
        maxvar = j;
      p[i++] = ivlc_key(&itr);
      p[i++] = (void *) (long) ivlc_value(&itr);
    }
  claim(i == 2 * n);
  ivlc_reset(poly);

  svlen = iv_length(perm);
  perm->length = perm_group(perm);
  ok = _mult_ps(p, n, maxvar, perm, rank, poly);
  perm->length = svlen;

  for (i = 0; i < n; i++)
    iv_free(p[2*i]);
  ml_free(p);

  if (ok != 0)
    {
      ivlc_free_all(poly);
      return NULL;
    }

  return poly;
}

static int
_mult_ps(void **poly, int n, int maxvar, ivector *perm, int rank,
         ivlincomb *res)
{
  int i, j, c, lnxx, mv0, mv1, ok;
  ivlincomb *res1;
  void *cc;

  if (maxvar == 0)
    {
      ivector *w = iv_new_copy(perm);  /* FIXME: OPTIMIZE! */
      if (w == NULL)
        return -1;
      c = (int) (long) poly[1];
      return ivlc_add_element(res, c, w, iv_hash(w), LC_FREE_ZERO);
    }

  mv0 = 0;
  mv1 = 0;
  j = 0;
  for (i = 0; i < n; i++)
    {
      ivector *xx = poly[2*i];
      lnxx = iv_length(xx);
      if (lnxx < maxvar)
        {
          if (mv0 < lnxx)
            mv0 = lnxx;
        }
      else
        {
          iv_elem(xx, maxvar - 1)--;
          while (lnxx > 0 && iv_elem(xx, lnxx-1) == 0)
            lnxx--;
          xx->length = lnxx;
          if (mv1 < lnxx)
            mv1 = lnxx;
          poly[2*i] = poly[2*j];
          poly[2*j] = xx;
          cc = poly[2*i+1];
          poly[2*i+1] = poly[2*j+1];
          poly[2*j+1] = cc;
          j++;
        }
    }

  res1 = ivlc_new(IVLC_HASHTABLE_SZ, IVLC_ARRAY_SZ);
  if (res1 == NULL)
    return -1;
  ok = _mult_ps(poly, j, mv1, perm, rank, res1);
  if (ok == 0)
    ok = _monk_add(maxvar, res1, rank, res);
  ivlc_free_all(res1);

  if (ok == 0 && j < n)
    ok = _mult_ps(poly + 2*j, n - j, mv0, perm, rank, res);
  return ok;
}


ivlincomb *mult_schubert(ivector *w1, ivector *w2, int rank)
{
  ivlincomb *poly, *lc;
  ivector *tmp;
  int svlen1, svlen2, w1len, w2len, t;

  w1len = perm_length(w1);
  w2len = perm_length(w2);
  if (w1len > w2len)
    {
      tmp = w1;
      w1 = w2;
      w2 = tmp;
      t = w1len;
      w1len = w2len;
      w2len = t;
    }

  svlen1 = iv_length(w1);
  svlen2 = iv_length(w2);
  w1->length = perm_group(w1);
  w2->length = perm_group(w2);
  lc = NULL;

  if (rank == 0)
    {
      rank = (((unsigned) -1) >> 1);
    }
  else if (2*(w1len + w2len) > rank*(rank-1) || bruhat_zero(w1, w2, rank))
    {
      lc = ivlc_new(IVLC_HASHTABLE_SZ, IVLC_ARRAY_SZ);
      goto free_return;
    }

  poly = trans(w1, 0);
  if (poly == NULL) goto free_return;
  lc = mult_poly_schubert(poly, w2, rank);

 free_return:
  w1->length = svlen1;
  w2->length = svlen2;
  return lc;
}


ivlincomb *mult_schubert_str(ivector *str1, ivector *str2)
{
  ivector *dv, *w1, *w2;
  ivlincomb *lc, *res;
  ivlc_iter itr;

  claim(str_iscompat(str1, str2));
  dv = w1 = w2 = NULL;
  lc = res = NULL;

  dv = str2dimvec(str1);
  if (dv == NULL) goto mult_str_fail;
  w1 = string2perm(str1);
  if (w1 == NULL) goto mult_str_fail;
  w2 = string2perm(str2);
  if (w2 == NULL) goto mult_str_fail;

  lc = mult_schubert(w1, w2, iv_length(w1));
  if (lc == NULL) goto mult_str_fail;

  iv_free(w1);
  iv_free(w2);
  w1 = w2 = NULL;

  res = ivlc_new(IVLC_HASHTABLE_SZ, IVLC_ARRAY_SZ);
  if (res == NULL) goto mult_str_fail;
  for (ivlc_first(lc, &itr); ivlc_good(&itr); ivlc_next(&itr))
    {
      ivector *str = perm2string(ivlc_key(&itr), dv);
      if (str == NULL) goto mult_str_fail;
      if (ivlc_insert(res, str, iv_hash(str), ivlc_value(&itr)) == NULL)
        {
          iv_free(str);
          goto mult_str_fail;
        }
    }

  ivlc_free_all(lc);
  iv_free(dv);
  return res;

 mult_str_fail:
  if (dv) iv_free(dv);
  if (w1) iv_free(w1);
  if (w2) iv_free(w2);
  if (lc) ivlc_free_all(lc);
  if (res) ivlc_free_all(res);
  return NULL;
}
