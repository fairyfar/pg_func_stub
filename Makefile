# contrib/pg_func_stub/Makefile

MODULE_big = pg_func_stub
OBJS = \
	$(WIN32RES) \
	pg_func_stub.o \
	pg_func_stub_inner.o

EXTENSION = pg_func_stub
DATA = pg_func_stub--1.0.sql
PGFILEDESC = "pg_func_stub - function stub for PostgreSQL"

LDFLAGS_SL += $(filter -lm, $(LIBS))

# Disabled because these tests require "shared_preload_libraries=pg_func_stub",
# which typical installcheck users do not have (e.g. buildfarm clients).
NO_INSTALLCHECK = 1

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/pg_stub_stub
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
