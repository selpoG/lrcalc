bin_PROGRAMS = lrcalc schubmult
check_PROGRAMS = allparts allperms allstrings test_partiter \
  test_schubmult test_lrmult test_lrcoef test_lrskew test_fusion

lrcalc_LDADD = liblrcalc.la
schubmult_LDADD = liblrcalc.la

allparts_LDADD = liblrcalc.la
allperms_LDADD = liblrcalc.la
allstrings_LDADD = liblrcalc.la

test_partiter_LDADD = liblrcalc.la
test_schubmult_LDADD = liblrcalc.la
test_lrmult_LDADD = liblrcalc.la
test_lrcoef_LDADD = liblrcalc.la
test_lrskew_LDADD = liblrcalc.la
test_fusion_LDADD = liblrcalc.la

lib_LTLIBRARIES = liblrcalc.la
liblrcalc_la_SOURCES = alloc.c ilist.c ivector.c ivlincomb.c ivlist.c lrcoef.c\
  lriter.c maple.c optshape.c part.c perm.c schublib.c schur.c vectarg.c

liblrcalc_la_LDFLAGS = $(LIBLRCALC_LDFLAGS) -version-info 2:0:0

lrcalcincludedir = $(includedir)/lrcalc
lrcalcinclude_HEADERS = *.h
