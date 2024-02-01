[中文](README-中文.md) | [English](README.md)

===========================================

`pg_func_stub` - `PostgreSQL`桩函数插件

===========================================

# 1. 警告
本插件主要用于调试或测试，获得使用权的用户可以对目标系统数据和程序进行修改，因此，请勿在有敏感数据的系统上使用！

# 2. 作用
利用本插件，可以在当前`PostgreSQL`会话中使用桩函数机制替换运行期函数，以达到不停止数据库服务，且不修改原始程序的情况下，替换现有函数功能。

# 3. 原理

关于桩函数的原理请参考：

[https://github.com/coolxv/cpp-stub](https://github.com/coolxv/cpp-stub)

本项目桩函数功能基于`cpp-stub`，原生`cpp-stub`是`C++`语言开发的，本项目进行了修改，改写成`C`语言程序。

扩展提供三个接口`SQL`函数，分别为：

- `pg_func_stub_set`：打桩
- `pg_func_stub_reset`：拆除指定的桩
- `pg_func_stub_clean`：拆除所有桩

请注意，扩展提供的三个接口函数，只是一个演示，您需要根据实际需求修改为现实功能。

# 4. 编译

以`Linux`系统`gcc`编译为例：

```bash
# 将pg_func_stub项目的所有文件拷贝到PostgreSQL源码的contrib目录下
[yz@localhost pg_func_stub]$ pwd
/home/yz/postgresql/contrib/pg_func_stub

[yz@localhost pg_func_stub]$ make clean
[yz@localhost pg_func_stub]$ make
[yz@localhost pg_func_stub]$ make install
```

# 5. 示例

```sql
CREATE EXTENSION pg_func_stub;
SELECT pg_func_stub_set('');
```

在`Linux`系统中，执行上述`SQL`后，我们将使用自己的桩函数替换原生`PostgreSQL`两个函数：

- `pg_cancel_backend`替换为`stub_demo_pg_cancel_backend`。
- `pg_terminate_backend`替换为`stub_demo_pg_terminate_backend`。

# 6. 开源协议

本项目遵循MIT开源协议。

Scripted by FairyFar. www.200yi.com
