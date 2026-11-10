AM_CFLAGS = $(libplist_CFLAGS) $(libtatsu_CFLAGS) $(libcurl_CFLAGS) $(zlib_CFLAGS) $(libcrypto_CFLAGS) $(libirecovery_CFLAGS) -I$(top_srcdir)/external/jssy/jssy/ -I$(top_srcdir)/external/libfragmentzip/include -I$(top_srcdir)/external/libgeneral/include -I$(top_builddir)/external/libgeneral/include
AM_CXXFLAGS = $(AM_CFLAGS)
AM_LDFLAGS = $(libplist_LIBS) $(libtatsu_LIBS) $(libcurl_LIBS) $(zlib_LIBS) $(libcrypto_LIBS) $(wolfssl_LIBS) $(libirecovery_LIBS) -lm

lib_LIBRARIES = libjssy.a libgeneral.a libfragmentzip.a
libjssy_a_CFLAGS = $(AM_CFLAGS)
libjssy_a_SOURCES = $(top_srcdir)/external/jssy/jssy/jssy.c

# Vendored libgeneral (external/libgeneral, submodule). Only the minimal
# exception.cpp object is built (matching upstream --without-extras /
# BUILD_MINIMAL); it pairs with the generated macros.h header. The archive
# is statically linked, no system install is needed.
libgeneral_a_CXXFLAGS = $(AM_CFLAGS)
libgeneral_a_SOURCES = $(top_srcdir)/external/libgeneral/libgeneral/exception.cpp

# Vendored libfragmentzip (external/libfragmentzip, submodule). Single C
# source, statically linked (no system libfragmentzip.pc needed).
libfragmentzip_a_CFLAGS = $(AM_CFLAGS)
libfragmentzip_a_SOURCES = $(top_srcdir)/external/libfragmentzip/libfragmentzip/libfragmentzip.c

if NOMAIN
noinst_LTLIBRARIES = libtsschecker.la
libtsschecker_la_SOURCES = tsschecker.c download.c common.c
libtsschecker_la_CFLAGS = $(AM_CFLAGS)
libtsschecker_la_LIBADD = $(AM_LDFLAGS) libjssy.a libgeneral.a libfragmentzip.a
else
bin_PROGRAMS = tsschecker
tsschecker_CFLAGS = $(AM_CFLAGS)
tsschecker_LDADD = $(AM_LDFLAGS) libjssy.a libgeneral.a libfragmentzip.a

tsschecker_SOURCES = tsschecker.c download.c common.c main.c
endif

if WINDOWS
tsschecker_LDFLAGS = -lpthread -Wl,--allow-multiple-definition -lbcrypt $(AM_LDFLAGS)
else
tsschecker_LDFLAGS = $(AM_LDFLAGS)
endif
