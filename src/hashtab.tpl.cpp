/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

SIZE_T PREFIX(card)(const HASHTAB* ht) { return ht->card; }

/* Initialize hash table structure. */
int PREFIX(init)(HASHTAB* ht, SIZE_T tabsz, SIZE_T eltsz)
{
	ht->card = 0;
	ht->free_elts = 0;
	ht->elts_len = 1;
	ht->table_sz = tabsz;
	ht->table = static_cast<SIZE_T*>(ml_calloc(tabsz, sizeof(SIZE_T)));
	if (ht->table == nullptr) return -1;
	ht->elts_sz = eltsz;
	ht->elts = static_cast<PREFIX(keyval_t)*>(ml_malloc(eltsz * sizeof(PREFIX(keyval_t))));
	if (ht->elts == nullptr)
	{
		ml_free(ht->table);
		return -1;
	}
	return 0;
}

HASHTAB* PREFIX(new)(SIZE_T tabsz, SIZE_T eltsz)
{
	auto ht = static_cast<HASHTAB*>(ml_malloc(sizeof(HASHTAB)));
	if (ht == nullptr) return nullptr;
	if (PREFIX(init)(ht, tabsz, eltsz) != 0)
	{
		ml_free(ht);
		return nullptr;
	}
	return ht;
}

void PREFIX(dealloc)(HASHTAB* ht)
{
	ml_free(ht->table);
	ml_free(ht->elts);
}

void PREFIX(free)(HASHTAB* ht)
{
	ml_free(ht->table);
	ml_free(ht->elts);
	ml_free(ht);
}

void PREFIX(reset)(HASHTAB* ht)
{
	memset(ht->table, 0, ht->table_sz * sizeof(SIZE_T));
	ht->card = 0;
	ht->free_elts = 0;
	ht->elts_len = 1;
}

int PREFIX(_grow_table)(HASHTAB* ht, SIZE_T sz)
{
	SIZE_T newsz = 2 * USE_FACTOR * sz + 1;
	if (newsz % 3 == 0) newsz += 2;
	if (newsz % 5 == 0) newsz += 6;
	if (newsz % 7 == 0) newsz += 30;
	auto newtab = static_cast<SIZE_T*>(ml_calloc(newsz, sizeof(SIZE_T)));
	if (newtab == nullptr) return -1;

	SIZE_T* oldtab = ht->table;
	PREFIX(keyval_t)* elts = ht->elts;
	SIZE_T next;
	for (SIZE_T index = 0; index < ht->table_sz; index++)
		for (SIZE_T i = oldtab[index]; i != 0; i = next)
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
	auto elts = static_cast<PREFIX(keyval_t)*>(ml_realloc(ht->elts, newsz * sizeof(PREFIX(keyval_t))));
	if (elts == nullptr) return -1;
	ht->elts_sz = newsz;
	ht->elts = elts;
	return 0;
}

int PREFIX(makeroom)(HASHTAB* ht, SIZE_T sz)
{
	if (USE_FACTOR * sz > ht->table_sz)
	{
		if (PREFIX(_grow_table)(ht, sz) != 0) return -1;
	}
	/* First entry of ht->elts not used. */
	if (sz + 1 > ht->elts_sz)
	{
		if (PREFIX(_grow_elts)(ht, sz + 1) != 0) return -1;
	}
	return 0;
}

/* Return pointer to keyval_t, nullptr if key not in table. */
PREFIX(keyval_t) * PREFIX(lookup)(const HASHTAB* ht, const KEY_T key, HASH_T hash)
{
	PREFIX(keyval_t)* elts = ht->elts;
	SIZE_T index = hash % ht->table_sz;
	SIZE_T i = ht->table[index];
	while (i != 0 && KEY_CMP(key, elts[i].key) != 0) i = ht->elts[i].next;
	return (i == 0) ? nullptr : elts + i;
}

/* Call only if key is not in table.  Insert key into table and return
   a pointer to new value variable, nullptr if memory allocation
   error. */
PREFIX(keyval_t) * PREFIX(insert)(HASHTAB* ht, KEY_T key, HASH_T hash, VALUE_T value)
{
	if (PREFIX(makeroom)(ht, ht->card + 1) != 0) return nullptr;
	ht->card++;
	PREFIX(keyval_t)* elts = ht->elts;
	SIZE_T i;
	if (ht->free_elts != 0)
	{
		i = ht->free_elts;
		ht->free_elts = elts[i].next;
	}
	else
	{
		i = ht->elts_len++;
	}
	PREFIX(keyval_t)* kvs = elts + i;
	kvs->key = key;
	kvs->hash = hash;
	kvs->value = value;
	SIZE_T index = hash % ht->table_sz;
	kvs->next = ht->table[index];
	ht->table[index] = i;
	return kvs;
}

/* Remove key from hashtable; return pointer to removed keyval_t, or nullptr. */
PREFIX(keyval_t) * PREFIX(remove)(HASHTAB* ht, const KEY_T key, HASH_T hash)
{
	PREFIX(keyval_t)* elts = ht->elts;
	SIZE_T* pi = ht->table + (hash % ht->table_sz);
	SIZE_T i = *pi;
	while (i != 0 && KEY_CMP(key, elts[i].key) != 0)
	{
		pi = &elts[i].next;
		i = *pi;
	}
	if (i == 0) return nullptr;
	ht->card--;
	*pi = elts[i].next;
	elts[i].next = ht->free_elts;
	ht->free_elts = i;
	return elts + i;
}

int PREFIX(equals)(const HASHTAB* ht1, const HASHTAB* ht2, int opt_zero)
{
	PREFIX(iter) itr;
	PREFIX(keyval_t) * kv1, *kv2;
	for (PREFIX(first)(ht1, &itr); PREFIX(good)(&itr); PREFIX(next)(&itr))
	{
		kv1 = PREFIX(keyval)(&itr);
		if (kv1->value == 0 && opt_zero == 0) continue;
		kv2 = PREFIX(lookup)(ht2, kv1->key, kv1->hash);
		if (kv2 == nullptr || kv1->value != kv2->value) return 0;
	}
	for (PREFIX(first)(ht2, &itr); PREFIX(good)(&itr); PREFIX(next)(&itr))
	{
		kv2 = PREFIX(keyval)(&itr);
		if (kv2->value == 0 && opt_zero == 0) continue;
		kv1 = PREFIX(lookup)(ht1, kv2->key, kv2->hash);
		if (kv1 == nullptr || kv1->value != kv2->value) return 0;
	}
	return 1;
}

int PREFIX(good)(const PREFIX(iter) * itr) { return (itr->i != 0); }

void PREFIX(first)(const HASHTAB* ht, PREFIX(iter) * itr)
{
	itr->ht = ht;
	SIZE_T index = 0;
	while (index < ht->table_sz && ht->table[index] == 0) index++;
	if (index == ht->table_sz)
	{
		itr->i = 0;
		return;
	}
	itr->index = index;
	itr->i = ht->table[index];
}

void PREFIX(next)(PREFIX(iter) * itr)
{
	const HASHTAB* ht = itr->ht;
	const PREFIX(keyval_t)* elts = ht->elts;
	if (elts[itr->i].next != 0)
	{
		itr->i = elts[itr->i].next;
		return;
	}
	SIZE_T index = SIZE_T(itr->index + 1);
	while (index < ht->table_sz && ht->table[index] == 0) index++;
	if (index == ht->table_sz)
	{
		itr->i = 0;
		return;
	}
	itr->index = index;
	itr->i = ht->table[index];
}

KEY_T PREFIX(key)(const PREFIX(iter) * itr) { return itr->ht->elts[itr->i].key; }

VALUE_T PREFIX(value)(const PREFIX(iter) * itr) { return itr->ht->elts[itr->i].value; }

PREFIX(keyval_t) * PREFIX(keyval)(const PREFIX(iter) * itr) { return itr->ht->elts + itr->i; }

void PREFIX(dealloc_refs)(HASHTAB* ht)
{
	PREFIX(iter) itr;
	for (PREFIX(first)(ht, &itr); PREFIX(good)(&itr); PREFIX(next)(&itr))
	{
		PREFIX(keyval_t)* kv = PREFIX(keyval)(&itr);
		KEY_DEALLOC(kv->key);
		VALUE_DEALLOC(kv->value);
	}
}

void PREFIX(dealloc_all)(HASHTAB* ht)
{
	PREFIX(dealloc_refs)(ht);
	PREFIX(dealloc)(ht);
}

void PREFIX(free_all)(HASHTAB* ht)
{
	PREFIX(dealloc_refs)(ht);
	PREFIX(free)(ht);
}

int PREFIX(add_element)(HASHTAB* ht, VALUE_T c, KEY_T key, HASH_T hash, int opt)
{
	if (c == 0)
	{
		if (!(opt & LC_COPY_KEY)) KEY_DEALLOC(key);
		return 0;
	}
	PREFIX(keyval_t)* kv = PREFIX(lookup)(ht, key, hash);
	if (kv != nullptr)
	{
		if (!(opt & LC_COPY_KEY)) KEY_DEALLOC(key);
		kv->value += c;
		if (kv->value == 0 && (opt & LC_FREE_ZERO))
		{
			PREFIX(remove)(ht, kv->key, hash);
			KEY_DEALLOC(kv->key);
		}
		return 0;
	}
	if (PREFIX(makeroom)(ht, ht->card + 1) != 0)
	{
		if (!(opt & LC_COPY_KEY)) KEY_DEALLOC(key);
		return -1;
	}
	if (opt & LC_COPY_KEY)
	{
		key = KEY_COPY(key);
		if (key == nullptr) return -1;
	}
	kv = PREFIX(insert)(ht, key, hash, c);
	return 0;
}

int PREFIX(add_multiple)(HASHTAB* dst, VALUE_T c, const HASHTAB* src, int opt)
{
	PREFIX(iter) itr;
	for (PREFIX(first)(src, &itr); PREFIX(good)(&itr); PREFIX(next)(&itr))
	{
		PREFIX(keyval_t)* kv = PREFIX(keyval)(&itr);
		if (PREFIX(add_element)(dst, c * kv->value, kv->key, kv->hash, opt) != 0) return -1;
	}
	return 0;
}

#ifdef HASHTAB_LINCOMB
void PREFIX(print)(const HASHTAB* ht, int opt_zero)
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

void PREFIX(print_stat)(const HASHTAB* ht)
{
	constexpr SIZE_T range = 20;
	SIZE_T stat[range];

	memset(stat, 0, range * sizeof(SIZE_T));

	SIZE_T cmp = 0;
	SIZE_T used = 0;
	for (SIZE_T index = 0; index < ht->table_sz; index++)
	{
		SIZE_T i = ht->table[index];
		if (i == 0) continue;
		used++;
		SIZE_T count = 0;
		while (i != 0)
		{
			count++;
			i = ht->elts[i].next;
		}
		cmp += (count + 1) * count / 2;
		SIZE_T c = (count > range) ? range : count;
		stat[c - 1] += count;
	}

	printf("Hash table size: %lu\n", static_cast<unsigned long>(ht->table_sz));
	printf("Hash table used: %lu\n", static_cast<unsigned long>(used));
	printf("Total elements: %lu\n", static_cast<unsigned long>(ht->card));
	if (ht->card != 0) printf("Average compares: %f\n", double(cmp) / ht->card);
	printf("Table distribution:");
	for (SIZE_T i = 0; i < range; i++) printf(" %d", stat[i]);
	putchar('\n');
}
