#ifndef LRCALC_CPP_LIB_H
#define LRCALC_CPP_LIB_H

// This header should be included only from implementation sources, not other headers.
#ifdef __cplusplus

#include <memory>

#include "lrcalc/ivector.hpp"
#include "lrcalc/ivlincomb.hpp"
#include "lrcalc/ivlist.hpp"
#include "lrcalc/part.hpp"

struct iv_deleter
{
	void operator()(ivector* p) const { iv_free(p); }
};
struct ivl_deleter
{
	void operator()(ivlist* p) const { ivl_free_all(p); }
};
struct ivlc_deleter
{
	void operator()(ivlincomb* p) const { ivlc_free_all(p); }
};
using iv_ptr = std::unique_ptr<ivector, iv_deleter>;
using ivl_ptr = std::unique_ptr<ivlist, ivl_deleter>;
using ivlc_ptr = std::unique_ptr<ivlincomb, ivlc_deleter>;

inline iv_ptr iv_create(uint32_t len) { return iv_ptr{iv_new(len)}; }

struct pitr
{
	pitr(ivector& p, int rows, int cols, const ivector* outer, const ivector* inner, int size, int opt)
	{
		pitr_first(itr, p, rows, cols, outer, inner, size, opt);
	}
	auto begin() const { return *this; }
	auto end() const { return nullptr; }
	pitr& operator++()
	{
		pitr_next(itr);
		return *this;
	}
	part_iter& operator*() { return itr; }
	bool operator==([[maybe_unused]] nullptr_t _) { return !pitr_good(itr); }

private:
	part_iter itr{};
};

#endif

#endif
