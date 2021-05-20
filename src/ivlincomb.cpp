/*  Littlewood-Richardson Calculator
 *  Copyright (C) 1999- Anders S. Buch (asbuch at math rutgers edu)
 *  See the file LICENSE for license information.
 */

#include "lrcalc/ivlincomb.hpp"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <new>

#include "lrcalc/cpp_lib.hpp"
#include "lrcalc/ivector.hpp"

uint32_t ivlc_card(const ivlincomb* ht) { return ht->card; }

/* Initialize hash table structure. */
static int ivlc_init(ivlincomb* ht, uint32_t tabsz, uint32_t eltsz)
{
	ht->card = 0;
	ht->free_elts = 0;
	ht->elts_len = 1;
	ht->table_sz = tabsz;
	ht->table = new (std::nothrow) uint32_t[tabsz]();
	if (ht->table == nullptr) return -1;
	ht->elts_sz = eltsz;
	ht->elts = new (std::nothrow) ivlc_keyval_t[eltsz];
	if (ht->elts == nullptr)
	{
		delete[] ht->table;
		return -1;
	}
	return 0;
}

ivlincomb* ivlc_new(uint32_t tabsz, uint32_t eltsz)
{
	auto ht = new (std::nothrow) ivlincomb;
	if (ht == nullptr) return nullptr;
	if (ivlc_init(ht, tabsz, eltsz) != 0)
	{
		delete ht;
		return nullptr;
	}
	return ht;
}

void ivlc_free(ivlincomb* ht)
{
	delete[] ht->table;
	delete[] ht->elts;
	delete ht;
}

void ivlc_reset(ivlincomb* ht)
{
	memset(ht->table, 0, ht->table_sz * sizeof(uint32_t));
	ht->card = 0;
	ht->free_elts = 0;
	ht->elts_len = 1;
}

static int ivlc_grow_table(ivlincomb* ht, uint32_t sz)
{
	uint32_t newsz = 2 * USE_FACTOR * sz + 1;
	if (newsz % 3 == 0) newsz += 2;
	if (newsz % 5 == 0) newsz += 6;
	if (newsz % 7 == 0) newsz += 30;
	auto newtab = new (std::nothrow) uint32_t[newsz]();
	if (newtab == nullptr) return -1;

	uint32_t* oldtab = ht->table;
	ivlc_keyval_t* elts = ht->elts;
	uint32_t next;
	for (uint32_t index = 0; index < ht->table_sz; index++)
		for (uint32_t i = oldtab[index]; i != 0; i = next)
		{
			uint32_t newidx = elts[i].hash % newsz;
			next = elts[i].next;
			elts[i].next = newtab[newidx];
			newtab[newidx] = i;
		}

	ht->table_sz = newsz;
	ht->table = newtab;
	delete[] oldtab;
	return 0;
}

static int ivlc_grow_elts(ivlincomb* ht, uint32_t sz)
{
	uint32_t newsz = 2 * sz;
	auto elts = new (std::nothrow) ivlc_keyval_t[newsz];
	if (elts == nullptr) return -1;
	memcpy(elts, ht->elts, ht->elts_len * sizeof(ivlc_keyval_t));
	delete[] ht->elts;
	ht->elts_sz = newsz;
	ht->elts = elts;
	return 0;
}

static int ivlc_makeroom(ivlincomb* ht, uint32_t sz)
{
	if (USE_FACTOR * sz > ht->table_sz)
	{
		if (ivlc_grow_table(ht, sz) != 0) return -1;
	}
	/* First entry of ht->elts not used. */
	if (sz + 1 > ht->elts_sz)
	{
		if (ivlc_grow_elts(ht, sz + 1) != 0) return -1;
	}
	return 0;
}

/* Return pointer to keyval_t, nullptr if key not in table. */
ivlc_keyval_t* ivlc_lookup(const ivlincomb* ht, const ivector* key, uint32_t hash)
{
	ivlc_keyval_t* elts = ht->elts;
	uint32_t index = hash % ht->table_sz;
	uint32_t i = ht->table[index];
	while (i != 0 && iv_cmp(key, elts[i].key) != 0) i = ht->elts[i].next;
	return (i == 0) ? nullptr : elts + i;
}

/* Call only if key is not in table.  Insert key into table and return
   a pointer to new value variable, nullptr if memory allocation
   error. */
ivlc_keyval_t* ivlc_insert(ivlincomb* ht, ivector* key, uint32_t hash, int32_t value)
{
	if (ivlc_makeroom(ht, ht->card + 1) != 0) return nullptr;
	ht->card++;
	ivlc_keyval_t* elts = ht->elts;
	uint32_t i;
	if (ht->free_elts != 0)
	{
		i = ht->free_elts;
		ht->free_elts = elts[i].next;
	}
	else
	{
		i = ht->elts_len++;
	}
	ivlc_keyval_t* kvs = elts + i;
	kvs->key = key;
	kvs->hash = hash;
	kvs->value = value;
	uint32_t index = hash % ht->table_sz;
	kvs->next = ht->table[index];
	ht->table[index] = i;
	return kvs;
}

/* Remove key from hashtable; return pointer to removed keyval_t, or nullptr. */
static ivlc_keyval_t* ivlc_remove(ivlincomb* ht, const ivector* key, uint32_t hash)
{
	ivlc_keyval_t* elts = ht->elts;
	uint32_t* pi = ht->table + (hash % ht->table_sz);
	uint32_t i = *pi;
	while (i != 0 && iv_cmp(key, elts[i].key) != 0)
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

bool ivlc_equals(const ivlincomb* ht1, const ivlincomb* ht2)
{
	for (const auto& kv1 : ivlc_iterator(ht1))
	{
		if (kv1.value == 0) continue;
		const ivlc_keyval_t* kv2 = ivlc_lookup(ht2, kv1.key, kv1.hash);
		if (kv2 == nullptr || kv1.value != kv2->value) return false;
	}
	for (const auto& kv2 : ivlc_iterator(ht2))
	{
		if (kv2.value == 0) continue;
		const ivlc_keyval_t* kv1 = ivlc_lookup(ht1, kv2.key, kv2.hash);
		if (kv1 == nullptr || kv1->value != kv2.value) return false;
	}
	return true;
}

static void ivlc_dealloc_refs(ivlincomb* ht)
{
	for (auto& kv : ivlc_iterator(ht)) iv_free(kv.key);
}

void ivlc_free_all(ivlincomb* ht)
{
	ivlc_dealloc_refs(ht);
	ivlc_free(ht);
}

// return true if secceeded
bool ivlc_add_element(ivlincomb* ht, int32_t c, ivector* key, uint32_t hash, int opt)
{
	if (c == 0)
	{
		if (!(opt & LC_COPY_KEY)) iv_free(key);
		return true;
	}
	ivlc_keyval_t* kv = ivlc_lookup(ht, key, hash);
	if (kv != nullptr)
	{
		if (!(opt & LC_COPY_KEY)) iv_free(key);
		kv->value += c;
		if (kv->value == 0 && (opt & LC_FREE_ZERO))
		{
			ivlc_remove(ht, kv->key, hash);
			iv_free(kv->key);
		}
		return true;
	}
	if (ivlc_makeroom(ht, ht->card + 1) != 0)
	{
		if (!(opt & LC_COPY_KEY)) iv_free(key);
		return false;
	}
	if (opt & LC_COPY_KEY) key = iv_new_copy(key);
	kv = ivlc_insert(ht, key, hash, c);
	return true;
}

// return true if secceeded
bool ivlc_add_multiple(ivlincomb* dst, int32_t c, ivlincomb* src, int opt)
{
	for (auto& kv : ivlc_iterator(src))
		if (!ivlc_add_element(dst, c * kv.value, kv.key, kv.hash, opt)) return false;
	return true;
}
