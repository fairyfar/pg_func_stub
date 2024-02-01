[中文](README-中文.md) | [English](README.md)

===========================================

`pg_func_stub` - stub function extension for `PostgreSQL`

===========================================

# 1. Warning

This extension is used for debugging or testing. Users who have obtained the right to use it can modify the target system data and programs. Therefore, please DO NOT use it on systems with sensitive data!

# 2. Feature

By using this extension, it is possible to replace runtime functions in the current `PostgreSQL` session using the `stub function` mechanism, in order to replace existing functions without stopping database services and modifying the original program.

# 3. Principle

For the principle of `stub function`, please refer to:

[https://github.com/coolxv/cpp-stub](https://github.com/coolxv/cpp-stub)

The extension of this project is based on `cpp_stub` project, and the original `cpp_stub` is developed in `C++` language. This project has been modified and rewritten with `C` language.

The extension provides three interface `SQL` functions:

- `pg_func_stub_set`: Set stub function.
- `pg_func_stub_reset`: Reset stub function.
- `pg_func_stub_clean`: Remove all stub function.

Please note that the three interface functions provided by the extension are only a demonstration, and you need to modify them to practical functionality according to actual requirements.

# 4. Compile

As an example, compile the code with `gcc` on `Linux`:

```bash
# Copy all files of the project files to the contrib directory of the PostgreSQL source codes.
[yz@localhost pg_func_stub]$ pwd
/home/yz/postgresql/contrib/pg_func_stub

[yz@localhost pg_func_stub]$ make clean
[yz@localhost pg_func_stub]$ make
[yz@localhost pg_func_stub]$ make install
```

# 5. Example

```SQL
CREATE EXTENSION pg_func_stub;
SELECT pg_func_stub_set('');
```

On `Linux` system, after executing the above `SQL`, we will replace the native `PostgreSQL` two functions with our own stub functions:

- ` pg_cancel_backend` stubbed with `stub_demo_pg_cancel_backend`.
- ` pg_terminate_backend` stubbed with `stub_demo_pg_terminate_backend`.

# 6. Open Source License

This project is released under the MIT license.

Scripted by FairyFar. www.200yi.com

