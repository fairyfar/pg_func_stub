#include "postgres.h"
#include "pg_func_stub_inner.h"
#include "funcapi.h"
#include "miscadmin.h"
#include "utils/builtins.h"

#ifndef _WIN32 
//linux or mac
#include <dlfcn.h>
#endif

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(pg_func_stub_set);
PG_FUNCTION_INFO_V1(pg_func_stub_reset);
PG_FUNCTION_INFO_V1(pg_func_stub_clean);

/*---- Demo functions ----*/

static Datum
stub_demo_pg_cancel_backend(PG_FUNCTION_ARGS)
{
	ereport(WARNING, (errmsg("pg_cancel_backend was stubbed by stub_demo_pg_cancel_backend")));
	PG_RETURN_BOOL(true);
}

static Datum
stub_demo_pg_terminate_backend(PG_FUNCTION_ARGS)
{
	ereport(WARNING, (errmsg("pg_terminate_backend was stubbed by stub_demopg_terminate_backend")));
	PG_RETURN_BOOL(true);
}

static bool
demo_stub_set(const char *func_name)
{
	void *dlhandle;
	void *iptr;
	bool ret = false;

	/* Demo 1: repace with function pointor */
	ret = _pg_func_stub_set("pg_cancel_backend", 
							(char *)pg_cancel_backend,
							(char *)stub_demo_pg_cancel_backend);
	if (!ret)
	{
		return false;
	}

	/* Demo 2: repace with function name */
#ifndef _WIN32
	// Linux ...
	dlhandle = dlopen(NULL, RTLD_LOCAL | RTLD_LAZY);
	if (!dlhandle)
	{
		ereport(ERROR,
				(errcode(ERRCODE_SYSTEM_ERROR),
				 errmsg("stub dlopen faild")));
	}
	iptr = dlsym(dlhandle, "pg_terminate_backend");
	if (!iptr)
	{
		ereport(ERROR,
				(errcode(ERRCODE_SYSTEM_ERROR),
				 errmsg("stub dlsym faild")));
	}
	ret = _pg_func_stub_set("pg_terminate_backend",
							(char *)iptr,
							(char *)stub_demo_pg_terminate_backend);
	if (!ret)
	{
		return false;
	}
#endif
	return ret;
}

static bool
demo_stub_reset(const char *func_name)
{
	bool ret = false;
	ret = _pg_func_stub_reset("pg_cancel_backend");
	if (!ret)
	{
		return false;
	}
	ret = _pg_func_stub_reset("pg_terminate_backend");
	if (!ret)
	{
		return false;
	}
	return ret;
}

/*---- Module functions ----*/

/*
 * Module load callback
 */
void
_PG_init(void)
{
	_pg_func_stub_init();
}

/*---- Module functions ----*/

Datum
pg_func_stub_set(PG_FUNCTION_ARGS)
{
	bool ret = false;
	const char *func_name = text_to_cstring(PG_GETARG_TEXT_PP(0));
	ret = demo_stub_set(func_name);
	PG_RETURN_BOOL(ret);
}

Datum
pg_func_stub_reset(PG_FUNCTION_ARGS)
{
	bool ret = false;
	const char *func_name = text_to_cstring(PG_GETARG_TEXT_PP(0));
	ret = demo_stub_reset(func_name);
	PG_RETURN_BOOL(ret);
}

Datum
pg_func_stub_clean(PG_FUNCTION_ARGS)
{
	_pg_func_stub_clear();
	PG_RETURN_BOOL(true);
}
