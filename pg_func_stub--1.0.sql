/* contrib/pg_func_stub/pg_func_stub--1.0.sql */

-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION pg_func_stub" to load this file. \quit

CREATE FUNCTION pg_func_stub_set(IN func TEXT)
RETURNS BOOL
AS 'MODULE_PATHNAME', 'pg_func_stub_set'
LANGUAGE C IMMUTABLE PARALLEL UNSAFE;

CREATE FUNCTION pg_func_stub_reset(IN func TEXT)
RETURNS BOOL
AS 'MODULE_PATHNAME', 'pg_func_stub_reset'
LANGUAGE C IMMUTABLE PARALLEL UNSAFE;

CREATE FUNCTION pg_func_stub_clean()
RETURNS BOOL
AS 'MODULE_PATHNAME', 'pg_func_stub_clean'
LANGUAGE C IMMUTABLE PARALLEL UNSAFE;
