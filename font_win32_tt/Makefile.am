## this needs more work, and is intended to work in a unix cross 
## compilation toolchain, or in something like msys

if ENABLE_WIN32_TT
aggincludedir = $(includedir)/agg2
agginclude_HEADERS = agg_font_win32_tt.h
noinst_LTLIBRARIES = libaggfontwin32tt.la

libaggfontwin32tt_la_LDFLAGS = -version-info @AGG_LIB_VERSION@ @WINDOWS_LIBS@ 
libaggfontwin32tt_la_SOURCES = agg_font_win32_tt.cpp
libaggfontwin32tt_la_CXXFLAGS = -I$(top_srcdir)/include @WINDOWS_CFLAGS@ 
endif

