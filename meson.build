# Copyright (c) 2024, fairyfar@msn.com

pg_func_stub_sources = files(
  'pg_func_stub_inner.c',
  'pg_func_stub.c',
)

if host_system == 'windows'
  pg_func_stub_sources += rc_lib_gen.process(win32ver_rc, extra_args: [
    '--NAME', 'pg_func_stub',
    '--FILEDESC', 'pg_func_stub - function stub for PostgreSQL',])
endif

pg_func_stub = shared_module('pg_func_stub',
  pg_func_stub_sources,
  kwargs: contrib_mod_args + {
    'dependencies': contrib_mod_args['dependencies'],
  },
)
contrib_targets += pg_func_stub

install_data(
  'pg_func_stub.control',
  'pg_func_stub--1.0.sql',
  kwargs: contrib_data_args,
)
