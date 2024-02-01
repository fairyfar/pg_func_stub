#include "postgres.h"
#include "pg_func_stub_inner.h"
#include "miscadmin.h"
#include "utils/hsearch.h"
#include "utils/memutils.h"

static long pg_stub_pagesize = -1;
static HTAB *pg_stub_htab = NULL;

static void *
pageof(char *addr)
{
#ifdef _WIN32
	return (void *)((unsigned long long)addr & ~(pg_stub_pagesize - 1));
#else
	return (void *)((unsigned long)addr & ~(pg_stub_pagesize - 1));
#endif
}

static bool
distanceof(char *addr, char *addr_stub)
{
	long int diff = addr_stub >= addr ? addr_stub - addr : addr - addr_stub;
	if ((sizeof(addr) > 4) && (((diff >> 31) - 1) > 0))
	{
		return true;
	}
	return false;
}

void
_pg_func_stub_init(void)
{
#ifdef _WIN32
	SYSTEM_INFO sys_info;
	GetSystemInfo(&sys_info);
	pg_stub_pagesize = sys_info.dwPageSize;
#else
	pg_stub_pagesize = sysconf(_SC_PAGE_SIZE);
#endif

	if (pg_stub_pagesize < 0)
	{
		pg_stub_pagesize = 4096;
	}

	if (pg_stub_htab == NULL)
	{
		HASHCTL ctl;
		memset(&ctl, 0, sizeof(ctl));
		ctl.keysize = sizeof(char *);
		ctl.entrysize = sizeof(func_stub);
		ctl.hcxt = TopMemoryContext;
		pg_stub_htab = hash_create("Func Stub", 8, &ctl, HASH_ELEM | HASH_BLOBS | HASH_CONTEXT);
	}
}

void
_pg_func_stub_clear(void)
{
	func_stub *pstub;
	HASH_SEQ_STATUS status;
	hash_seq_init(&status, pg_stub_htab);

	while ((pstub = (func_stub *)hash_seq_search(&status)) != NULL)
	{
#ifdef _WIN32
		DWORD lpflOldProtect;
		if (0 != VirtualProtect(pageof(pstub->fn), pg_stub_pagesize * 2, PAGE_EXECUTE_READWRITE, &lpflOldProtect))
#else
		if (0 == mprotect(pageof(pstub->fn), pg_stub_pagesize * 2, PROT_READ | PROT_WRITE | PROT_EXEC))
#endif
		{

			if (pstub->far_jmp)
			{
				memcpy(pstub->fn, pstub->code_buf, CODESIZE_MAX);
			}
			else
			{
				memcpy(pstub->fn, pstub->code_buf, CODESIZE_MIN);
			}

			CACHEFLUSH((char *)pstub->fn, CODESIZE);

#ifdef _WIN32
			VirtualProtect(pageof(pstub->fn), pg_stub_pagesize * 2, PAGE_EXECUTE_READ, &lpflOldProtect);
#else
			mprotect(pageof(pstub->fn), pg_stub_pagesize * 2, PROT_READ | PROT_EXEC);
#endif
		}
	}

	hash_destroy(pg_stub_htab);
}

bool
_pg_func_stub_set(char *key, char *fn, char *fn_stub)
{
	func_stub pstub_obj;
	func_stub *pstub = &pstub_obj;
	bool found;

	if (!superuser())
		ereport(ERROR,
				(errcode(ERRCODE_INSUFFICIENT_PRIVILEGE),
				 errmsg("must be superuser to use the functions")));

	// reset
	_pg_func_stub_reset(key);
	pstub = hash_search(pg_stub_htab, &key, HASH_ENTER, &found);
	pstub->key = key;
	pstub->fn = fn;

	if (distanceof(fn, fn_stub))
	{
		pstub->far_jmp = true;
		memcpy(pstub->code_buf, fn, CODESIZE_MAX);
	}
	else
	{
		pstub->far_jmp = false;
		memcpy(pstub->code_buf, fn, CODESIZE_MIN);
	}

#ifdef _WIN32
	DWORD lpflOldProtect;
	if (0 == VirtualProtect(pageof(pstub->fn), pg_stub_pagesize * 2, PAGE_EXECUTE_READWRITE, &lpflOldProtect))
#else
	if (-1 == mprotect(pageof(pstub->fn), pg_stub_pagesize * 2, PROT_READ | PROT_WRITE | PROT_EXEC))
#endif
	{
		hash_search(pg_stub_htab, &key, HASH_REMOVE, NULL);
		ereport(LOG,
				(errcode(ERRCODE_SYSTEM_ERROR),
				 errmsg("stub set memory protect to w+r+x faild")));
		return false;
	}

	if (pstub->far_jmp)
	{
		REPLACE_FAR(fn, fn_stub);
	}
	else
	{
		REPLACE_NEAR(fn, fn_stub);
	}

#ifdef _WIN32
	if (0 == VirtualProtect(pageof(pstub->fn), pg_stub_pagesize * 2, PAGE_EXECUTE_READ, &lpflOldProtect))
#else
	if (-1 == mprotect(pageof(pstub->fn), pg_stub_pagesize * 2, PROT_READ | PROT_EXEC))
#endif
	{
		hash_search(pg_stub_htab, &key, HASH_REMOVE, NULL);
		ereport(LOG,
				(errcode(ERRCODE_SYSTEM_ERROR),
				 errmsg("stub set memory protect to r+x failed")));
		return false;
	}

	return true;
}

bool
_pg_func_stub_reset(char *key)
{
	func_stub *pstub;
	bool found;

	if (!superuser())
		ereport(ERROR,
				(errcode(ERRCODE_INSUFFICIENT_PRIVILEGE),
				 errmsg("must be superuser to use the functions")));

	pstub = hash_search(pg_stub_htab, &key, HASH_FIND, &found);
	if (!found)
	{
		return false;
	}

#ifdef _WIN32
	DWORD lpflOldProtect;
	if (0 == VirtualProtect(pageof(pstub->fn), pg_stub_pagesize * 2, PAGE_EXECUTE_READWRITE, &lpflOldProtect))
#else
	if (-1 == mprotect(pageof(pstub->fn), pg_stub_pagesize * 2, PROT_READ | PROT_WRITE | PROT_EXEC))
#endif
	{
		ereport(LOG,
				(errcode(ERRCODE_SYSTEM_ERROR),
				 errmsg("stub reset memory protect to w+r+x faild")));
		return false;
	}

	if (pstub->far_jmp)
	{
		memcpy(pstub->fn, pstub->code_buf, CODESIZE_MAX);
	}
	else
	{
		memcpy(pstub->fn, pstub->code_buf, CODESIZE_MIN);
	}

	CACHEFLUSH((char *)pstub->fn, CODESIZE);

#ifdef _WIN32
	if (0 == VirtualProtect(pageof(pstub->fn), pg_stub_pagesize * 2, PAGE_EXECUTE_READ, &lpflOldProtect))
#else
	if (-1 == mprotect(pageof(pstub->fn), pg_stub_pagesize * 2, PROT_READ | PROT_EXEC))
#endif
	{
		ereport(LOG,
				(errcode(ERRCODE_SYSTEM_ERROR),
				 errmsg("stub reset memory protect to r+x failed")));
		return false;
	}

	hash_search(pg_stub_htab, &key, HASH_REMOVE, NULL);
	return true;
}
