/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

int PREFIX(_grow_table)(HASHTAB* ht, SIZE_T sz)
{
	SIZE_T *oldtab, *newtab;
	PREFIX(keyval_t) * elts;
	SIZE_T newsz, index, i, next;

	newsz = 2 * USE_FACTOR * sz + 1;
	if (newsz % 3 == 0) newsz += 2;
	if (newsz % 5 == 0) newsz += 6;
	if (newsz % 7 == 0) newsz += 30;
	newtab = (SIZE_T*)ml_calloc(newsz, sizeof(SIZE_T));
	if (newtab == NULL) return -1;

	oldtab = ht->table;
	elts = ht->elts;
	for (index = 0; index < ht->table_sz; index++)
		for (i = oldtab[index]; i != 0; i = next)
		{
			SIZE_T newidx = elts[i].hash % newsz;
			next = elts[i].next;
			elts[i].next = newtab[newidx];
			newtab[newidx] = i;
		}

	ht->table_sz = newsz;
	ht->table = newtab;
	ml_free(oldtab);
	return 0;
}

int PREFIX(_grow_elts)(HASHTAB* ht, SIZE_T sz)
{
	SIZE_T newsz = 2 * sz;
	PREFIX(keyval_t)* elts = (PREFIX(keyval_t)*)ml_realloc(ht->elts, newsz * sizeof(PREFIX(keyval_t)));
	if (elts == NULL) return -1;
	ht->elts_sz = newsz;
	ht->elts = elts;
	return 0;
}

int PREFIX(equals)(HASHTAB* ht1, HASHTAB* ht2, int opt_zero)
{
	PREFIX(iter) itr;
	PREFIX(keyval_t) * kv1, *kv2;
	for (PREFIX(first)(ht1, &itr); PREFIX(good)(&itr); PREFIX(next)(&itr))
	{
		kv1 = PREFIX(keyval)(&itr);
		if (kv1->value == 0 && opt_zero == 0) continue;
		kv2 = PREFIX(lookup)(ht2, kv1->key, kv1->hash);
		if (kv2 == NULL || kv1->value != kv2->value) return 0;
	}
	for (PREFIX(first)(ht2, &itr); PREFIX(good)(&itr); PREFIX(next)(&itr))
	{
		kv2 = PREFIX(keyval)(&itr);
		if (kv2->value == 0 && opt_zero == 0) continue;
		kv1 = PREFIX(lookup)(ht1, kv2->key, kv2->hash);
		if (kv1 == NULL || kv1->value != kv2->value) return 0;
	}
	return 1;
}

#ifdef HASHTAB_LINCOMB
void PREFIX(print)(HASHTAB* ht, int opt_zero)
{
	PREFIX(iter) itr;
	for (PREFIX(first)(ht, &itr); PREFIX(good)(&itr); PREFIX(next)(&itr))
	{
		if (PREFIX(value)(&itr) == 0 && opt_zero == 0) continue;
		printf("%d  ", PREFIX(value)(&itr));
		KEY_PRINT(PREFIX(key)(&itr));
		putchar('\n');
	}
}
#endif

void PREFIX(print_stat)(HASHTAB* ht)
{
	SIZE_T index, used, cmp, count, c, i;
	const SIZE_T range = 20;
	SIZE_T stat[range];

	memset(stat, 0, range * sizeof(SIZE_T));

	cmp = 0;
	used = 0;
	for (index = 0; index < ht->table_sz; index++)
	{
		i = ht->table[index];
		if (i == 0) continue;
		used++;
		count = 0;
		while (i != 0)
		{
			count++;
			i = ht->elts[i].next;
		}
		cmp += (count + 1) * count / 2;
		c = (count > range) ? range : count;
		stat[c - 1] += count;
	}

	printf("Hash table size: %lu\n", (unsigned long)ht->table_sz);
	printf("Hash table used: %lu\n", (unsigned long)used);
	printf("Total elements: %lu\n", (unsigned long)ht->card);
	if (ht->card != 0) printf("Average compares: %f\n", ((double)cmp) / ht->card);
	printf("Table distribution:");
	for (i = 0; i < range; i++) printf(" %d", stat[i]);
	putchar('\n');
}